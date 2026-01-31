/**
 * Flutter UVC Frame Renderer
 *
 * Captures raw frames using uvc_get_frame() and renders to both:
 * 1. Flutter texture (preview)
 * 2. MediaCodec surface (recording)
 *
 * Copyright (c) 2024 Statslane
 * Based on saki4510t's UVC4Flutter architecture
 */

#ifndef FLUTTER_UVC_FRAME_RENDERER_H
#define FLUTTER_UVC_FRAME_RENDERER_H

// Standard C/C++ headers
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Android headers
#include <android/native_window.h>

// Project headers
#include "aandusb/aandusb_native.h"

namespace serenegiant::flutter {

/**
 * Callback for notifying when a frame is available
 */
using FrameCallback =
    std::function<void(const uint8_t *data, size_t len, uint32_t width,
                       uint32_t height, int64_t pts_us)>;

/**
 * Frame renderer class that captures UVC frames and renders to multiple outputs
 */
class FlutterUvcFrameRenderer {
public:
  /**
   * Constructor
   * @param manager USB manager pointer
   * @param device_id UVC device ID
   */
  FlutterUvcFrameRenderer(usb_manager_t *manager, int32_t device_id);

  ~FlutterUvcFrameRenderer();

  // Disable copy
  FlutterUvcFrameRenderer(const FlutterUvcFrameRenderer &) = delete;
  FlutterUvcFrameRenderer &operator=(const FlutterUvcFrameRenderer &) = delete;

  /**
   * Start the frame capture and rendering loop
   * @param width Video width
   * @param height Video height
   * @param frame_type Frame type (MJPEG, YUY2, etc.)
   * @return 0 on success, negative on error
   */
  int start(uint32_t width, uint32_t height, uint32_t frame_type);

  /**
   * Stop the frame capture loop
   */
  void stop();

  /**
   * Check if running
   */
  bool isRunning() const { return m_is_running; }

  /**
   * Set the preview window (Flutter texture surface)
   * @param window Native window for preview
   */
  void setPreviewWindow(ANativeWindow *window);

  /**
   * Set the recording window (MediaCodec input surface)
   * @param window Native window for recording
   */
  void setRecordingWindow(ANativeWindow *window);

  /**
   * Set frame callback for additional processing
   */
  void setFrameCallback(FrameCallback callback);

  /**
   * Get current frame rate (calculated)
   */
  float getFrameRate() const;

  /**
   * Get frame count since start
   */
  int64_t getFrameCount() const { return m_frame_count; }

private:
  usb_manager_t *m_manager;
  int32_t m_device_id;

  // Video parameters
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_frame_type;

  // State
  std::atomic<bool> m_is_running{false};
  std::thread m_capture_thread;
  std::mutex m_mutex;

  // Output windows
  ANativeWindow *m_preview_window;
  ANativeWindow *m_recording_window;

  // Frame callback
  FrameCallback m_frame_callback;

  // Statistics
  std::atomic<int64_t> m_frame_count{0};
  int64_t m_start_time_ns;

  // Frame buffer
  std::vector<uint8_t> m_frame_buffer;
  std::vector<uint8_t> m_rgb_buffer;

  /**
   * Main capture loop - runs on separate thread
   */
  void captureLoop();

  /**
   * Render frame to a native window
   */
  void renderToWindow(ANativeWindow *window, const uint8_t *data,
                      uint32_t width, uint32_t height);

  /**
   * Convert YUV to RGB for display
   */
  void convertYuvToRgb(const uint8_t *yuv, uint8_t *rgb, uint32_t width,
                       uint32_t height);
};

} // namespace serenegiant::flutter

#endif // FLUTTER_UVC_FRAME_RENDERER_H
