/**
 * aAndUsb
 * Copyright (c) 2014-2026 saki t_saki@serenegiant.com
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

#define LOG_TAG "FlutterPluginMain"

#if 1 // デバッグ情報を出さない時は1
#ifndef LOG_NDEBUG
#define LOG_NDEBUG // LOGV/LOGD/MARKを出力しない時
#endif
#undef USE_LOGALL // 指定したLOGxだけを出力
#ifndef LOG_NDEBUG
#define LOG_NDEBUG // LOGV/LOGD/MARKを出力しない時
#endif
#else
#define USE_LOGALL
#define USE_LOGD
#undef LOG_NDEBUG
#undef NDEBUG
#endif

// android
#include <android/native_window.h>
#include <android/native_window_jni.h>
// dart
#include "../dartAPIDL/dart_api_dl.h"
// aandusb
#include "utilbase.h"
// common
#include "common/jni_utils.h"
// flutter
#include "flutter_plugin.h"
#include "flutter_plugin_java.h"

// Java側オブジェクトのFQCN
#define FQCN_JAVA_PLUGIN "com/serenegiant/flutter/uvcplugin/UVCManager"

namespace plugin = serenegiant::flutter;
namespace sere = serenegiant;

static std::mutex plugin_lock;
static plugin::FlutterPluginJavaUp pluginJava;

//--------------------------------------------------------------------------------
// DartのFlutterプラグイン部分から呼ばれる関数

DART_EXPORT
int32_t get_state(int32_t device_id)
{
  ENTER();

  LOGV("id=%d", device_id);
  device_state_t result = UNINITIALIZED;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->get_device_state(device_id);
  }

  RETURN(result, int32_t);
}

DART_EXPORT
int32_t get_device_info(const int32_t device_id, usb_device_info_t *info_out)
{
  ENTER();

  LOGV("id=%d", device_id);
  int32_t result = -1;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava && info_out)
  {
    *info_out = pluginJava->get_device_info(device_id);
    result = 0;
  }

  RETURN(result, int);
}

DART_EXPORT
int64_t start(const int32_t device_id)
{
  ENTER();

  LOGV("id=%d", device_id);
  int64_t result = -1;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->start(device_id);
  }

  RETURN(result, int64_t);
}

DART_EXPORT
int32_t stop(const int32_t device_id)
{
  ENTER();

  LOGV("id=%d", device_id);
  int32_t result = -1;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->stop(device_id);
  }

  RETURN(result, int);
}

DART_EXPORT
int set_video_size(const int32_t device_id, const uint32_t type,
                   const uint32_t width, const uint32_t height)
{

  ENTER();

  LOGV("id=%d", device_id);
  int result = -1;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->set_video_size(device_id, (uvc_raw_frame_t)type, width,
                                        height);
  }

  RETURN(result, int);
}

DART_EXPORT
int get_current_size(int32_t device_id, uvc_video_size_t *data)
{

  ENTER();

  LOGV("id=%d", device_id);
  int result = -1;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava && data)
  {
    result = pluginJava->get_current_size(device_id, data);
  }

  RETURN(result, int);
}

/**
 * コントロール機能でサポートしている機能を取得
 * @param device_id
 * @return
 */
DART_EXPORT
uint64_t get_ctrl_supports(int32_t device_id)
{
  ENTER();

  uint64_t result = 0;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->get_ctrl_supports(device_id);
  }

  RETURN(result, int32_t);
}

/**
 * プロセッシングユニットでサポートしている機能を取得
 * @param device_id
 * @return
 */
DART_EXPORT
uint64_t get_proc_supports(int32_t device_id)
{
  ENTER();

  uint64_t result = 0;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->get_proc_supports(device_id);
  }

  RETURN(result, int32_t);
}

/**
 * 指定した機能の設定情報を取得
 * @param device_id
 * @param value
 * @return
 */
DART_EXPORT
int32_t get_ctrl_info(int32_t device_id, uvc_control_info_t *value)
{
  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->get_control_info(device_id, *value);
  }

  RETURN(result, int32_t);
}

/**
 * 指定した機能の設定値を適用
 * @param device_id
 * @param type
 * @param value
 * @return
 */
DART_EXPORT
int32_t set_ctrl_value(int32_t device_id, uint64_t type, int32_t value)
{
  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->set_control_value(device_id, type, value);
  }

  RETURN(result, int32_t);
}

/**
 * 指定した機能の設定値を取得
 * @param device_id
 * @param type
 * @param value
 * @return
 */
DART_EXPORT
int32_t get_ctrl_value(int32_t device_id, uint64_t type, int32_t *value)
{
  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result = pluginJava->get_control_value(device_id, type, *value);
  }

  RETURN(result, int32_t);
}

/**
 * native側でUVC映像サイズ設定へアクセスするときのヘルパー関数
 * 主にUnityやFlutterからのアクセスを想定
 * @param device_id
 * @param index 映像サイズ設定のインデックス
 * @param num_supported
 * 対応している映像サイズ設定の数を入れるuint32_tへのポインタ
 * @param data
 * 映像サイズ設定を書き込むためのunity_video_size_t構造体へのポインタ
 * @return 0: 成功, 負: エラーコード
 */
DART_EXPORT
int32_t get_supported_size(int32_t device_id, int32_t index,
                           int32_t *num_supported, uvc_video_size_t *data)
{

  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    result =
        pluginJava->get_supported_size(device_id, index, num_supported, data);
  }

  RETURN(result, int32_t);
}

/**
 * 映像取得用のsurfaceをセットする
 * @param device_id UVC機器の識別子
 * @param tex_id   テクスチャID
 * @param jsurface Java側のSurfaceオブジェクト
 */
DART_EXPORT
int32_t set_preview_surface(int32_t device_id, // jint
                            int64_t tex_id,    // jlong
                            void *jsurface)
{ // jobject jsurface

  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    const auto is_available = pluginJava->is_available(device_id);
    if (is_available)
    {
      if (jsurface)
      {
        serenegiant::AutoJNIEnv _env;
        auto env = _env.get();
        auto *window = ANativeWindow_fromSurface(env, (jobject)jsurface);
        result = pluginJava->set_preview_window(device_id, tex_id, window);
      }
      else
      {
        result = pluginJava->set_preview_window(device_id, tex_id, nullptr);
      }
    }
  }

  RETURN(result, int32_t);
}

//--------------------------------------------------------------------------------
// JavaのFlutterプラグインオブジェクト(UVCManager)から呼ばれる

static int nativeInit(JNIEnv *env, jobject thiz)
{
  ENTER();

  jobject _thiz = env->NewGlobalRef(thiz);
  LOGD("create FlutterPluginJava");
  auto p = std::make_unique<plugin::FlutterPluginJava>(_thiz);
  {
    std::lock_guard<std::mutex> lock(plugin_lock);
    pluginJava = std::move(p);
  }
  LOGD("FlutterPluginJava=%p", pluginJava.get());

  RETURN(0, int);
}

static int nativeSetSurface(JNIEnv *env, jobject, jint deviceId, jlong texId,
                            jobject jsurface)
{

  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    const auto is_available = pluginJava->is_available(deviceId);
    if (is_available)
    {
      if (jsurface)
      {
        LOGD("set surface");
        auto *window = ANativeWindow_fromSurface(env, (jobject)jsurface);
        result = pluginJava->set_preview_window(deviceId, texId, window);
      }
      else
      {
        LOGD("clear surface");
        result = pluginJava->set_preview_window(deviceId, texId, nullptr);
      }
    }
  }

  RETURN(result, int);
}

static int nativeRelease(JNIEnv *, jobject)
{
  ENTER();

  plugin::FlutterPluginJavaSp p;
  {
    std::lock_guard<std::mutex> lock(plugin_lock);
    p = std::move(pluginJava);
  }
  if (p)
  {
    LOGD("release FlutterPluginJava");
    p.reset();
  }

  RETURN(0, int);
}

//--------------------------------------------------------------------------------
// Recording functions - uses uvc_get_frame approach
// These are placeholder implementations that delegate to existing UVC functions
// The actual frame renderer integration happens at the Kotlin level

static jint nativeStartRecording(JNIEnv *env, jobject thiz, jint deviceId,
                                 jstring jpath, jint width, jint height,
                                 jint bitrate)
{

  ENTER();

  // Get path string
  const char *path = env->GetStringUTFChars(jpath, nullptr);
  LOGD("nativeStartRecording: device=%d, path=%s, %dx%d @ %d bps", deviceId,
       path, width, height, bitrate);

  // For now, just log and return success
  // The actual recording will be handled at the Kotlin level using MediaCodec
  // because we need the Surface from MediaCodec.createInputSurface()

  env->ReleaseStringUTFChars(jpath, path);

  RETURN(0, jint);
}

static jstring nativeStopRecording(JNIEnv *env, jobject thiz, jint deviceId)
{
  ENTER();

  LOGD("nativeStopRecording: device=%d", deviceId);

  // Return null for now - actual implementation at Kotlin level
  RET(nullptr);
}

static jboolean nativeIsRecording(JNIEnv *env, jobject thiz, jint deviceId)
{
  ENTER();

  // Return false for now
  RETURN(JNI_FALSE, jboolean);
}

static jint nativeSetRecordingSurface(JNIEnv *env, jobject thiz, jint deviceId,
                                      jlong surfaceHandle)
{

  ENTER();

  LOGD("nativeSetRecordingSurface (handle): device=%d, handle=%lld", deviceId,
       (long long)surfaceHandle);

  // For recording, we need to handle this differently
  // The frame renderer will write to this surface

  RETURN(0, jint);
}

/**
 * Set recording surface from Java Surface object
 * This is the primary method for connecting MediaCodec encoder surface
 * @param env JNI environment
 * @param thiz Java object
 * @param deviceId UVC device ID
 * @param jsurface Java Surface object (can be null to stop recording)
 * @return 0 on success, negative on error
 */
static jint nativeSetRecordingSurfaceObj(JNIEnv *env, jobject thiz, jint deviceId,
                                         jobject jsurface)
{
  ENTER();

  int32_t result = -5;
  std::lock_guard<std::mutex> lock(plugin_lock);
  if (pluginJava)
  {
    const auto is_available = pluginJava->is_available(deviceId);
    if (is_available)
    {
      if (jsurface)
      {
        LOGD("nativeSetRecordingSurfaceObj: set recording surface for device=%d", deviceId);
        auto *window = ANativeWindow_fromSurface(env, jsurface);
        result = pluginJava->set_recording_window(deviceId, window);
        // Note: window reference is managed by the holder, don't release here
      }
      else
      {
        LOGD("nativeSetRecordingSurfaceObj: clear recording surface for device=%d", deviceId);
        result = pluginJava->set_recording_window(deviceId, nullptr);
      }
    }
    else
    {
      LOGW("nativeSetRecordingSurfaceObj: device %d not available", deviceId);
    }
  }

  RETURN(result, jint);
}

//================================================================================
static JNINativeMethod methods[] = {
    {"nativeInit", "()I", (void *)nativeInit},
    {"nativeRelease", "()I", (void *)nativeRelease},

    {"nativeSetSurface", "(IJLandroid/view/Surface;)I",
     (void *)nativeSetSurface},

    // Recording methods
    {"nativeStartRecording", "(ILjava/lang/String;III)I",
     (void *)nativeStartRecording},
    {"nativeStopRecording", "(I)Ljava/lang/String;",
     (void *)nativeStopRecording},
    {"nativeIsRecording", "(I)Z", (void *)nativeIsRecording},
    {"nativeSetRecordingSurface", "(IJ)I", (void *)nativeSetRecordingSurface},
    {"nativeSetRecordingSurfaceObj", "(ILandroid/view/Surface;)I",
     (void *)nativeSetRecordingSurfaceObj},
};

int register_plugin(JNIEnv *env)
{
  ENTER();

  // ネイティブメソッドを登録
  if (sere::registerNativeMethods(env, FQCN_JAVA_PLUGIN, methods,
                                  NUM_ARRAY_ELEMENTS(methods)) < 0)
  {
    env->ExceptionClear();
    return -1;
  }

  RETURN(0, int);
}
