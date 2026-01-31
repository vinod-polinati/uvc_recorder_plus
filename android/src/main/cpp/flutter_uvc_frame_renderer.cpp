/**
 * Flutter UVC Frame Renderer Implementation
 *
 * Captures raw frames using uvc_get_frame() and renders to both:
 * 1. Flutter texture (preview)
 * 2. MediaCodec surface (recording)
 *
 * Copyright (c) 2024 Statslane
 * Based on saki4510t's UVC4Flutter architecture
 */

#define LOG_TAG "FlutterUvcFrameRenderer"

#if 1
#ifndef LOG_NDEBUG
#define LOG_NDEBUG
#endif
#undef USE_LOGALL
#else
#define USE_LOGALL
#undef LOG_NDEBUG
#undef NDEBUG
#endif

// Standard C/C++ headers (first to avoid conflicts)
#include <algorithm>
#include <chrono>

// Android headers
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

// Project headers
#include "flutter_uvc_frame_renderer.h"
#include "utilbase.h"

// Helper macros for logging when utilbase.h macros aren't working
#ifndef LOGD
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGW
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGI
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif

namespace serenegiant::flutter {

//------------------------------------------------------------------------------
// Helper function to get current time in nanoseconds
//------------------------------------------------------------------------------
static int64_t getCurrentTimeNs() {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
FlutterUvcFrameRenderer::FlutterUvcFrameRenderer(usb_manager_t *manager,
                                                 int32_t device_id)
    : m_manager(manager), m_device_id(device_id), m_width(1280), m_height(720),
      m_frame_type(RAW_FRAME_MJPEG), m_preview_window(nullptr),
      m_recording_window(nullptr), m_start_time_ns(0) {
  LOGD("FlutterUvcFrameRenderer created for device %d", device_id);
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
FlutterUvcFrameRenderer::~FlutterUvcFrameRenderer() {
  LOGD("FlutterUvcFrameRenderer destructor");
  stop();
}

//------------------------------------------------------------------------------
// Start frame capture
//------------------------------------------------------------------------------
int FlutterUvcFrameRenderer::start(uint32_t width, uint32_t height,
                                   uint32_t frame_type) {
  LOGD("start: %dx%d, frame_type=%d", width, height, frame_type);

  if (m_is_running) {
    LOGW("Already running");
    return -1;
  }

  m_width = width;
  m_height = height;
  m_frame_type = frame_type;
  m_frame_count = 0;
  m_start_time_ns = getCurrentTimeNs();

  // Allocate frame buffers
  // For YUV420: width * height * 1.5
  // For YUYV: width * height * 2
  // For MJPEG: variable, allocate generously
  size_t buffer_size = width * height * 3; // Generous allocation
  m_frame_buffer.resize(buffer_size);
  m_rgb_buffer.resize(width * height * 4); // RGBA

  // Request video size from UVC device
  int result = uvc_resize(m_manager, m_device_id, frame_type, width, height);
  if (result != 0) {
    LOGE("Failed to set video size: %d", result);
    return -2;
  }

  // Start UVC streaming
  result = uvc_start(m_manager, m_device_id);
  if (result != 0) {
    LOGE("Failed to start UVC: %d", result);
    return -3;
  }

  m_is_running = true;

  // Start capture thread
  m_capture_thread = std::thread(&FlutterUvcFrameRenderer::captureLoop, this);

  LOGD("Frame capture started");
  return 0;
}

//------------------------------------------------------------------------------
// Stop frame capture
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::stop() {
  LOGD("stop");

  if (!m_is_running) {
    return;
  }

  m_is_running = false;

  // Wait for capture thread
  if (m_capture_thread.joinable()) {
    m_capture_thread.join();
  }

  // Stop UVC streaming
  uvc_stop(m_manager, m_device_id);

  // Clear buffers
  m_frame_buffer.clear();
  m_rgb_buffer.clear();

  LOGD("Frame capture stopped, frames: %lld", (long long)m_frame_count.load());
}

//------------------------------------------------------------------------------
// Set preview window
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::setPreviewWindow(ANativeWindow *window) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_preview_window) {
    ANativeWindow_release(m_preview_window);
  }

  m_preview_window = window;

  if (window) {
    ANativeWindow_acquire(window);
    // Set buffer format
    ANativeWindow_setBuffersGeometry(window, m_width, m_height,
                                     WINDOW_FORMAT_RGBA_8888);
  }

  LOGD("Preview window set: %p", window);
}

//------------------------------------------------------------------------------
// Set recording window
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::setRecordingWindow(ANativeWindow *window) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_recording_window) {
    ANativeWindow_release(m_recording_window);
  }

  m_recording_window = window;

  if (window) {
    ANativeWindow_acquire(window);
  }

  LOGD("Recording window set: %p", window);
}

//------------------------------------------------------------------------------
// Set frame callback
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::setFrameCallback(FrameCallback callback) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_frame_callback = callback;
}

//------------------------------------------------------------------------------
// Get frame rate
//------------------------------------------------------------------------------
float FlutterUvcFrameRenderer::getFrameRate() const {
  if (m_frame_count == 0 || m_start_time_ns == 0) {
    return 0.0f;
  }

  int64_t elapsed_ns = getCurrentTimeNs() - m_start_time_ns;
  if (elapsed_ns <= 0) {
    return 0.0f;
  }

  return (float)m_frame_count.load() * 1e9f / (float)elapsed_ns;
}

//------------------------------------------------------------------------------
// Main capture loop
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::captureLoop() {
  LOGD("Capture loop started");

  while (m_is_running) {
    // Get frame from UVC camera
    uint32_t frame_type = m_frame_type;
    uint32_t width = m_width;
    uint32_t height = m_height;
    uint32_t data_len = m_frame_buffer.size();
    int64_t pts_us = 0;
    uint32_t flags = 0;

    int result =
        uvc_get_frame(m_manager, m_device_id, &frame_type, &width, &height,
                      m_frame_buffer.data(), &data_len, &pts_us, &flags);

    if (result != 0 || data_len == 0) {
      // No frame available, wait a bit
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    m_frame_count++;

    // Convert to displayable format (YUV->RGB)
    convertYuvToRgb(m_frame_buffer.data(), m_rgb_buffer.data(), width, height);

    {
      std::lock_guard<std::mutex> lock(m_mutex);

      // Render to preview window
      if (m_preview_window) {
        renderToWindow(m_preview_window, m_rgb_buffer.data(), width, height);
      }

      // Render to recording window (if recording)
      if (m_recording_window) {
        renderToWindow(m_recording_window, m_rgb_buffer.data(), width, height);
      }

      // Call frame callback if set
      if (m_frame_callback) {
        m_frame_callback(m_frame_buffer.data(), data_len, width, height,
                         pts_us);
      }
    }

    // Log FPS periodically
    if (m_frame_count % 100 == 0) {
      LOGD("Frame %lld, FPS: %.1f", (long long)m_frame_count.load(),
           getFrameRate());
    }
  }

  LOGD("Capture loop ended");
}

//------------------------------------------------------------------------------
// Render frame to native window
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::renderToWindow(ANativeWindow *window,
                                             const uint8_t *data,
                                             uint32_t width, uint32_t height) {
  if (!window || !data) {
    return;
  }

  ANativeWindow_Buffer buffer;
  if (ANativeWindow_lock(window, &buffer, nullptr) != 0) {
    return;
  }

  // Copy RGBA data to window buffer
  uint8_t *dst = static_cast<uint8_t *>(buffer.bits);
  const uint8_t *src = data;

  int copy_width = std::min((int)width, buffer.width);
  int copy_height = std::min((int)height, buffer.height);

  for (int y = 0; y < copy_height; y++) {
    memcpy(dst, src, copy_width * 4);
    dst += buffer.stride * 4;
    src += width * 4;
  }

  ANativeWindow_unlockAndPost(window);
}

//------------------------------------------------------------------------------
// Convert YUV to RGB
// Supports YUYV and NV21 formats
//------------------------------------------------------------------------------
void FlutterUvcFrameRenderer::convertYuvToRgb(const uint8_t *yuv, uint8_t *rgb,
                                              uint32_t width, uint32_t height) {
  // For now, assume YUYV format (most common for UVC cameras)
  // YUYV: 2 pixels per 4 bytes (Y0 U0 Y1 V0)

  for (uint32_t i = 0; i < width * height / 2; i++) {
    int y0 = yuv[i * 4 + 0];
    int u = yuv[i * 4 + 1] - 128;
    int y1 = yuv[i * 4 + 2];
    int v = yuv[i * 4 + 3] - 128;

    // First pixel
    int r0 = y0 + (359 * v >> 8);
    int g0 = y0 - (88 * u + 183 * v >> 8);
    int b0 = y0 + (454 * u >> 8);

    rgb[i * 8 + 0] = std::clamp(r0, 0, 255);
    rgb[i * 8 + 1] = std::clamp(g0, 0, 255);
    rgb[i * 8 + 2] = std::clamp(b0, 0, 255);
    rgb[i * 8 + 3] = 255; // Alpha

    // Second pixel
    int r1 = y1 + (359 * v >> 8);
    int g1 = y1 - (88 * u + 183 * v >> 8);
    int b1 = y1 + (454 * u >> 8);

    rgb[i * 8 + 4] = std::clamp(r1, 0, 255);
    rgb[i * 8 + 5] = std::clamp(g1, 0, 255);
    rgb[i * 8 + 6] = std::clamp(b1, 0, 255);
    rgb[i * 8 + 7] = 255; // Alpha
  }
}

} // namespace serenegiant::flutter
