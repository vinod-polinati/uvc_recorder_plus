/**
 * Video Recorder for UVC Camera - Surface Copying Approach
 * 
 * Uses MediaCodec and MediaMuxer for hardware-accelerated H.264 encoding.
 * Receives frames via a callback and draws them to the encoder surface.
 */
package com.serenegiant.flutter.uvcplugin

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Rect
import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.media.MediaMuxer
import android.os.Handler
import android.os.HandlerThread
import android.os.SystemClock
import android.util.Log
import android.view.Surface
import androidx.annotation.Keep
import java.util.concurrent.atomic.AtomicBoolean

@Keep
class UvcVideoRecorder {
    companion object {
        private const val TAG = "UvcVideoRecorder"
        private const val DEBUG = true
        
        // H.264 encoding parameters
        private const val MIME_TYPE = MediaFormat.MIMETYPE_VIDEO_AVC
        private const val FRAME_RATE = 30
        private const val I_FRAME_INTERVAL = 1
        private const val DEFAULT_BITRATE = 4_000_000 // 4 Mbps
    }
    
    // Recording state
    private var isRecording = AtomicBoolean(false)
    private var outputPath: String? = null
    
    // Encoder components
    private var mediaCodec: MediaCodec? = null
    private var mediaMuxer: MediaMuxer? = null
    private var encoderSurface: Surface? = null
    private var videoTrackIndex = -1
    private var muxerStarted = false
    
    // Encoding thread
    private var encoderThread: HandlerThread? = null
    private var encoderHandler: Handler? = null
    
    // Timing
    private var startTimeNs = 0L
    private var frameCount = 0L
    private var lastFrameTimeNs = 0L
    private var frameIntervalNs = 1_000_000_000L / FRAME_RATE  // nanoseconds per frame
    
    // Dimensions
    private var videoWidth = 1280
    private var videoHeight = 720
    
    /**
     * Get the encoder input surface.
     * The UVC library should render frames to this surface for recording.
     */
    fun getEncoderSurface(): Surface? = encoderSurface
    
    /**
     * Check if currently recording
     */
    fun isRecording(): Boolean = isRecording.get()
    
    /**
     * Get frame count
     */
    fun getFrameCount(): Long = frameCount
    
    /**
     * Start recording to the specified path
     */
    fun startRecording(path: String, width: Int, height: Int, bitrate: Int = DEFAULT_BITRATE): Int {
        if (isRecording.get()) {
            Log.w(TAG, "Already recording")
            return -1
        }
        
        if (DEBUG) Log.d(TAG, "startRecording: $path ($width x $height) @ $bitrate bps")
        
        try {
            outputPath = path
            frameCount = 0
            startTimeNs = System.nanoTime()
            lastFrameTimeNs = startTimeNs
            
            // Ensure width/height are even
            videoWidth = if (width % 2 == 0) width else width + 1
            videoHeight = if (height % 2 == 0) height else height + 1
            
            // Create encoder thread
            encoderThread = HandlerThread("UvcEncoderThread").apply { start() }
            encoderHandler = Handler(encoderThread!!.looper)
            
            // Configure video format
            val format = MediaFormat.createVideoFormat(MIME_TYPE, videoWidth, videoHeight).apply {
                setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface)
                setInteger(MediaFormat.KEY_BIT_RATE, bitrate)
                setInteger(MediaFormat.KEY_FRAME_RATE, FRAME_RATE)
                setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, I_FRAME_INTERVAL)
            }
            
            // Create encoder
            mediaCodec = MediaCodec.createEncoderByType(MIME_TYPE).apply {
                configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)
                encoderSurface = createInputSurface()
                start()
            }
            
            // Create muxer
            mediaMuxer = MediaMuxer(path, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4)
            videoTrackIndex = -1
            muxerStarted = false
            
            isRecording.set(true)
            
            // Start drain loop
            encoderHandler?.post { drainEncoder() }
            
            // Note: Frames are now provided by native UVC renderer via encoder surface
            // No test frame generator needed - real camera frames are rendered directly
            // to encoderSurface by the native layer
            
            if (DEBUG) Log.d(TAG, "Recording started successfully, waiting for frames from native renderer")
            return 0
            
        } catch (e: Exception) {
            Log.e(TAG, "Failed to start recording", e)
            cleanup()
            return -2
        }
    }
    
    /**
     * Write a frame to the encoder.
     * Call this method with each preview frame bitmap.
     */
    fun writeFrame(bitmap: Bitmap) {
        if (!isRecording.get() || encoderSurface == null || !encoderSurface!!.isValid) {
            return
        }
        
        // Rate limiting - only record at target frame rate
        val now = System.nanoTime()
        if (now - lastFrameTimeNs < frameIntervalNs) {
            return
        }
        lastFrameTimeNs = now
        
        try {
            val canvas = encoderSurface!!.lockCanvas(null)
            if (canvas != null) {
                // Scale bitmap to fit encoder surface
                val srcRect = Rect(0, 0, bitmap.width, bitmap.height)
                val dstRect = Rect(0, 0, videoWidth, videoHeight)
                canvas.drawBitmap(bitmap, srcRect, dstRect, null)
                encoderSurface!!.unlockCanvasAndPost(canvas)
                frameCount++
                
                if (DEBUG && frameCount % 30 == 0L) {
                    Log.d(TAG, "Frame $frameCount written")
                }
            }
        } catch (e: Exception) {
            Log.w(TAG, "Failed to write frame", e)
        }
    }
    
    /**
     * Write a frame using direct surface drawing.
     * This is called with src surface coordinates to blit.
     */
    fun submitFrame() {
        if (!isRecording.get()) return
        
        val now = System.nanoTime()
        if (now - lastFrameTimeNs < frameIntervalNs) {
            return
        }
        lastFrameTimeNs = now
        frameCount++
    }
    
    /**
     * Stop recording and return the output file path
     */
    fun stopRecording(): String? {
        if (!isRecording.get()) {
            Log.w(TAG, "Not recording")
            return null
        }
        
        if (DEBUG) Log.d(TAG, "stopRecording, frames=$frameCount")
        
        isRecording.set(false)
        
        // Signal end of stream
        try {
            mediaCodec?.signalEndOfInputStream()
        } catch (e: Exception) {
            Log.w(TAG, "Error signaling EOS", e)
        }
        
        // Wait for encoder to finish
        Thread.sleep(500)
        
        cleanup()
        
        val path = outputPath
        outputPath = null
        
        if (DEBUG) Log.d(TAG, "Recording stopped: $path, total frames: $frameCount")
        return path
    }
    
    /**
     * Drain encoded buffers from the encoder
     */
    private fun drainEncoder() {
        if (DEBUG) Log.d(TAG, "drainEncoder started")
        
        val bufferInfo = MediaCodec.BufferInfo()
        
        while (isRecording.get() || !muxerStarted) {
            val codec = mediaCodec ?: break
            
            try {
                val index = codec.dequeueOutputBuffer(bufferInfo, 10000)
                
                when {
                    index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED -> {
                        if (!muxerStarted) {
                            val format = codec.outputFormat
                            videoTrackIndex = mediaMuxer?.addTrack(format) ?: -1
                            mediaMuxer?.start()
                            muxerStarted = true
                            if (DEBUG) Log.d(TAG, "Muxer started, track: $videoTrackIndex")
                        }
                    }
                    
                    index >= 0 -> {
                        val buffer = codec.getOutputBuffer(index)
                        
                        if (buffer != null && muxerStarted && bufferInfo.size > 0) {
                            buffer.position(bufferInfo.offset)
                            buffer.limit(bufferInfo.offset + bufferInfo.size)
                            
                            mediaMuxer?.writeSampleData(videoTrackIndex, buffer, bufferInfo)
                        }
                        
                        codec.releaseOutputBuffer(index, false)
                        
                        if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
                            if (DEBUG) Log.d(TAG, "End of stream reached")
                            break
                        }
                    }
                    
                    else -> {
                        // No buffer available, continue
                    }
                }
            } catch (e: Exception) {
                if (isRecording.get()) {
                    Log.e(TAG, "Drain error", e)
                }
                break
            }
        }
        
        if (DEBUG) Log.d(TAG, "drainEncoder finished")
    }
    
    /**
     * Clean up all resources
     */
    private fun cleanup() {
        if (DEBUG) Log.d(TAG, "cleanup")
        
        try {
            mediaCodec?.stop()
            mediaCodec?.release()
        } catch (e: Exception) {
            Log.w(TAG, "Codec cleanup error", e)
        }
        mediaCodec = null
        
        try {
            if (muxerStarted) {
                mediaMuxer?.stop()
            }
            mediaMuxer?.release()
        } catch (e: Exception) {
            Log.w(TAG, "Muxer cleanup error", e)
        }
        mediaMuxer = null
        muxerStarted = false
        
        encoderSurface?.release()
        encoderSurface = null
        
        encoderThread?.quitSafely()
        encoderThread = null
        encoderHandler = null
    }
}
