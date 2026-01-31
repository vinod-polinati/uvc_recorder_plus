/**
 * Copyright (c) 2020-2026 saki t_saki@serenegiant.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
package com.serenegiant.flutter.uvcplugin

import android.app.Activity
import android.util.Log
import android.util.LongSparseArray
import android.view.Surface
import android.view.WindowManager
import androidx.annotation.Keep
import androidx.core.util.forEach
import com.serenegiant.usb.DeviceDetector
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.embedding.engine.plugins.activity.ActivityAware
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.view.TextureRegistry
import java.lang.ref.WeakReference

/**
 * UVCManager
 */
@Keep
class UVCManager: FlutterPlugin, MethodCallHandler, ActivityAware {
  /// The MethodChannel that will the communication between Flutter and native Android
  ///
  /// This local reference serves to register the plugin with the Flutter Engine and unregister it
  /// when the Flutter Engine is detached from the Activity
  private lateinit var mChannel : MethodChannel
  private lateinit var mTextureRegistry: TextureRegistry
  private lateinit var mActivity: WeakReference<Activity>
  private val mSurfaceProducers = LongSparseArray<TextureRegistry.SurfaceProducer>()
  private var mNeedInitialize = false
  
  // Video recorder for UVC camera
  private var mVideoRecorder: UvcVideoRecorder? = null
  
  // Dual renderer for preview + recording
  private var mDualRenderer: GlDualRenderer? = null
  private var mPreviewSurfaceTexture: android.graphics.SurfaceTexture? = null

  override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    if (DEBUG) Log.v(TAG, "onAttachedToEngine:")
    nativeInit()
    mTextureRegistry = flutterPluginBinding.textureRegistry
    mChannel = MethodChannel(flutterPluginBinding.binaryMessenger, METHOD_CHANNEL_NAME)
    mChannel.setMethodCallHandler(this)
  }

  override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
    if (DEBUG) Log.v(TAG, "onDetachedFromEngine:")
    mChannel.setMethodCallHandler(null)
    nativeRelease()
  }

  override fun onAttachedToActivity(binding: ActivityPluginBinding) {
    if (DEBUG) Log.v(TAG, "onAttachedToActivity:")
    mActivity = WeakReference(binding.activity)
    if (mNeedInitialize) {
      mNeedInitialize = false
      // 多分無いと思うけどメソッドコールからの"initialize"が先に来てしまったとき
      if (DEBUG) Log.v(TAG, "onAttachedToActivity:initUVCDeviceDetector")
      DeviceDetector.initUVCDeviceDetector(binding.activity)
    }
  }

  override fun onDetachedFromActivityForConfigChanges() {
    if (DEBUG) Log.v(TAG, "onDetachedFromActivityForConfigChanges:")
  }

  override fun onReattachedToActivityForConfigChanges(binding: ActivityPluginBinding) {
    if (DEBUG) Log.v(TAG, "onReattachedToActivityForConfigChanges:")
    mActivity = WeakReference(binding.activity)
  }

  override fun onDetachedFromActivity() {
    if (DEBUG) Log.v(TAG, "onDetachedFromActivity:")
    mNeedInitialize = false
    releaseTextureAll()
    val a = mActivity.get()
    if (a != null) {
      if (DEBUG) Log.v(TAG, "onDetachedFromActivity:releaseDeviceDetector")
      DeviceDetector.releaseDeviceDetector(a)
    }
  }

  override fun onMethodCall(call: MethodCall, result: Result) {
    if (DEBUG) Log.v(TAG, "onMethodCall:${call.method}")
    when (call.method) {
      "initialize" -> {
        val a = mActivity.get()
        if (a != null) {
          if (DEBUG) Log.v(TAG, "onMethodCall#initialize:initUVCDeviceDetector")
          DeviceDetector.initUVCDeviceDetector(a)
        } else {
          mNeedInitialize = true
        }
      }
      "createTexture" -> {
        val deviceId: Int? = call.argument("deviceId")
        val width: Int? = call.argument("width")
        val height: Int? = call.argument("height")
        if ((deviceId != null) && (width != null) && (height != null)) {
          result.success(createTexture(deviceId, width, height))
        } else {
          result.error("failed to get deviceId/width/height", null, null)
        }
      }
      "releaseTexture" -> {
        val deviceId: Int? = call.argument("deviceId")
        val texId: Any? = call.argument("textureId")
        val textureId = if (texId is Int) texId.toLong() else (texId as Long?)
        if ((deviceId != null) && (textureId != null)) {
          releaseTexture(deviceId, textureId)
        }
        result.success(null)
      }
      "keepScreenOn" -> {
        val activity = mActivity.get()
        if (activity != null) {
          val window = activity.window
          val onoff: Boolean? = call.argument("onoff")
          if (window != null) {
            activity.runOnUiThread {
              if (onoff == true) {
                window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
              } else {
                window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
              }
            }
          }
          result.success(null)
          return
        }
        result.error("No Activity", null, null)
      }
      // Recording methods - using Kotlin MediaCodec recorder
      "startRecording" -> {
        val deviceId: Int? = call.argument("deviceId")
        val path: String? = call.argument("path")
        val width: Int = call.argument("width") ?: 1280
        val height: Int = call.argument("height") ?: 720
        val bitrate: Int = call.argument("bitrate") ?: 4000000
        if (deviceId != null && path != null) {
          try {
            if (mVideoRecorder == null) {
              mVideoRecorder = UvcVideoRecorder()
            }
            val r = mVideoRecorder?.startRecording(path, width, height, bitrate) ?: -1
            
            // Get the encoder surface and connect to native frame renderer
            val encoderSurface = mVideoRecorder?.getEncoderSurface()
            if (encoderSurface != null && r == 0) {
              // Connect encoder surface to native UVC holder for direct frame rendering
              val nativeResult = nativeSetRecordingSurfaceObj(deviceId, encoderSurface)
              if (DEBUG) Log.v(TAG, "nativeSetRecordingSurfaceObj result=$nativeResult")
              
              // Also connect to dual renderer as fallback
              mDualRenderer?.setRecordingSurface(encoderSurface)
              if (DEBUG) Log.v(TAG, "Encoder surface connected to native + dual renderer")
            }
            
            if (DEBUG) Log.v(TAG, "startRecording: deviceId=$deviceId, path=$path, result=$r")
            result.success(r)
          } catch (e: Exception) {
            Log.e(TAG, "Failed to start recording", e)
            result.error("RECORDING_ERROR", e.message, null)
          }
        } else {
          result.error("INVALID_ARGS", "deviceId and path required", null)
        }
      }
      "stopRecording" -> {
        val deviceId: Int? = call.argument("deviceId")
        if (deviceId != null) {
          try {
            // Disconnect recording surface from native and dual renderer
            val nativeResult = nativeSetRecordingSurfaceObj(deviceId, null)
            if (DEBUG) Log.v(TAG, "nativeSetRecordingSurfaceObj(null) result=$nativeResult")
            mDualRenderer?.setRecordingSurface(null)
            
            val path = mVideoRecorder?.stopRecording()
            if (DEBUG) Log.v(TAG, "stopRecording: deviceId=$deviceId, path=$path")
            result.success(path)
          } catch (e: Exception) {
            Log.e(TAG, "Failed to stop recording", e)
            result.error("RECORDING_ERROR", e.message, null)
          }
        } else {
          result.error("INVALID_ARGS", "deviceId required", null)
        }
      }
      "isRecording" -> {
        val deviceId: Int? = call.argument("deviceId")
        if (deviceId != null) {
          val recording = mVideoRecorder?.isRecording() ?: false
          result.success(recording)
        } else {
          result.error("INVALID_ARGS", "deviceId required", null)
        }
      }
      else -> {
        result.notImplemented()
      }
    }
  }

  //--------------------------------------------------------------------------------
  /**
   * Dart側からのメソッドコールの実体
   * Dart側のTextureで表示するためSurfaceTextureを生成しテクスチャidを返す
   * @param deviceId 対象とするUVC機器のid
   * @param width
   * @param height
   * @return テクスチャid
   */
  private fun createTexture(deviceId: Int, width: Int, height: Int): Long {
    if (DEBUG) Log.v(TAG, "createTexture:deviceId=${deviceId}/(${width}x${height})")
    try {
      val producer = mTextureRegistry.createSurfaceProducer()
      producer.setSize(width, height)
      mSurfaceProducers.append(producer.id(), producer)
      // native側へSurfaceをセット
      nativeSetSurface(deviceId, producer.id(), producer.surface)
      if (DEBUG) Log.v(TAG, "createTexture:producer=${producer}")
      return producer.id()
    } catch (e: Exception) {
      if (DEBUG) Log.w(TAG, e)
      throw e
    }
  }

  /**
   * Dart側からのメソッドコールの実体
   * Dart側でTextureを使って表示するのに使っていたテクスチャ/SurfaceTextureを破棄する
   * @param deviceId 対象とするUVC機器のid
   */
  private fun releaseTexture(deviceId: Int, textureId: Long) {
    if (DEBUG) Log.v(TAG, "releaseTexture:deviceId=${deviceId},textureId=${textureId}")
    nativeSetSurface(deviceId, -1, null)
    val producer = mSurfaceProducers.get(textureId)
    producer?.release()
    mSurfaceProducers.remove(textureId)
  }

  /**
   * テクスチャ/Surfaceが生成されていれば破棄する
   * activityからデタッチされるときに呼び出す
   */
  private fun releaseTextureAll() {
    if (DEBUG) Log.v(TAG, "releaseTextureAll:")
    mSurfaceProducers.forEach { _, producer ->
      producer.release()
    }
    mSurfaceProducers.clear()
  }

  @Keep
  external fun nativeInit(): Int

  @Keep
  external fun nativeRelease(): Int

  @Keep
  external fun nativeSetSurface(deviceId: Int, texId: Long, surface: Surface?): Int

  // Recording native methods
  @Keep
  external fun nativeStartRecording(deviceId: Int, path: String, width: Int, height: Int, bitrate: Int): Int

  @Keep
  external fun nativeStopRecording(deviceId: Int): String?

  @Keep
  external fun nativeIsRecording(deviceId: Int): Boolean

  @Keep
  external fun nativeSetRecordingSurface(deviceId: Int, surfaceHandle: Long): Int

  /**
   * Set recording surface from Java Surface object
   * This passes the MediaCodec encoder surface to native for direct frame rendering
   */
  @Keep
  external fun nativeSetRecordingSurfaceObj(deviceId: Int, surface: Surface?): Int

  companion object {
    private const val DEBUG = true // Enable debug logging for development
    private val TAG = UVCManager::class.java.simpleName

    private const val METHOD_CHANNEL_NAME = "com.serenegiant.flutter/aandusb_method"
    init {
      NativeLibLoader.loadNative()
    }
  }
}
