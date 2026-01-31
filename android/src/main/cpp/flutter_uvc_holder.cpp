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

#define LOG_TAG "FlutterUVCHolder"

#define COUNT_FRAMES (0)
#define MEAS_TIME (0) // 1フレーム当たりの描画時間を測定する時1

#if 1 // デバッグ情報を出さない時は1
#ifndef LOG_NDEBUG
#define LOG_NDEBUG // LOGV/LOGD/MARKを出力しない時
#endif
#undef USE_LOGALL // 指定したLOGxだけを出力
#else
#define USE_LOGALL
#undef LOG_NDEBUG
#undef NDEBUG
#endif

// Standard C++
#include <algorithm>
#include <chrono>

// aandusb
#include "utilbase.h"
// common
#include "common/eglbase.h"
// flutter
#include "flutter_uvc_holder.h"

//--------------------------------------------------------------------------------
#if MEAS_TIME
#define MEAS_TIME_INIT                         \
	static nsecs_t _meas_time_ = 0;            \
	static nsecs_t _init_time_ = systemTime(); \
	static int _meas_count_ = 0;
#define MEAS_TIME_START const nsecs_t _meas_t_ = systemTime();
#define MEAS_TIME_STOP                                                                \
	_meas_time_ += (systemTime() - _meas_t_);                                         \
	_meas_count_++;                                                                   \
	if (UNLIKELY((_meas_count_ % 100) == 0))                                          \
	{                                                                                 \
		const float d = _meas_time_ / (1000000.f * _meas_count_);                     \
		const float fps = _meas_count_ * 1000000000.f / (systemTime() - _init_time_); \
		LOGI("draw time=%5.2f[msec]/fps=%5.2f", d, fps);                              \
	}
#define MEAS_RESET    \
	_meas_count_ = 0; \
	_meas_time_ = 0;  \
	_init_time_ = systemTime();
#else
#define MEAS_TIME_INIT
#define MEAS_TIME_START
#define MEAS_TIME_STOP
#define MEAS_RESET
#endif

namespace serenegiant::flutter
{

	/*public*/
	FlutterUVCHolder::FlutterUVCHolder(
		usb_manager_t *manager, const int32_t &device_id,
		const uvc_raw_frame_t &frame_type,
		const uint32_t &width, const uint32_t &height)
		: m_manager(manager),
		  m_device_id(device_id),
		  m_current_size(),
		  m_supported_size(),
		  m_supported_ctrls()
	{
		ENTER();

		uvc_resize(m_manager, m_device_id, frame_type, width, height);

		// 対応解像度一覧を取得する
		int32_t num_supported = 0;
		auto r = uvc_get_supported_size(manager, m_device_id, 0, &num_supported, nullptr);
		if (!r && num_supported)
		{
			uvc_video_size_t size;
			for (int32_t i = 0; i < num_supported; i++)
			{
				r = uvc_get_supported_size(manager, m_device_id, i, &num_supported, &size);
				if (!r)
				{
					m_supported_size.push_back(size);
					LOGD("video_size_t(type=0x%08x,ix=%d,%dx%d)", size.frame_type, size.frame_index, size.width, size.height);
				}
				else
				{
					LOGE("Failed to get supported size,err=%d", r);
					break;
				}
			}
		}
		else
		{
			LOGE("Failed to get supported size,err=%d", r);
		}
		LOGD("num_supported=%d,added=%" FMT_SIZE_T, num_supported, m_supported_size.size());

		get_current_size();
		update_supported_ctrls();

		EXIT();
	}

	/*public*/
	FlutterUVCHolder::~FlutterUVCHolder() noexcept
	{
		ENTER();

		// Stop recording thread first
		if (m_recording_active)
		{
			m_recording_active = false;
			if (m_recording_thread && m_recording_thread->joinable())
			{
				m_recording_thread->join();
			}
			m_recording_thread.reset();
		}

		// Release recording window
		if (m_recording_window)
		{
			ANativeWindow_release(m_recording_window);
			m_recording_window = nullptr;
		}

		uvc_stop(m_manager, m_device_id);

		EXIT();
	}

	bool FlutterUVCHolder::is_running() const
	{
		ENTER();

		const auto state = uvc_get_device_state(m_manager, m_device_id);

		RETURN(state > CONNECTED, bool);
	}

	int FlutterUVCHolder::set_config(const int32_t &enabled, const bool &use_first_config)
	{
		ENTER();
		RETURN(uvc_set_config(m_manager, m_device_id, enabled, use_first_config), int);
	}

	uint64_t FlutterUVCHolder::get_ctrl_supports()
	{
		ENTER();
		RETURN(uvc_get_ctrl_supports(m_manager, m_device_id), uint64_t);
	}

	uint64_t FlutterUVCHolder::get_proc_supports()
	{
		ENTER();
		RETURN(uvc_get_proc_supports(m_manager, m_device_id), uint64_t);
	}

	/**
	 * UVC設定機能の情報を取得
	 * @param info
	 * @return 0: 成功, 負: エラーコード
	 */
	int FlutterUVCHolder::get_control_info(uvc_control_info_t &info) const
	{
		ENTER();
		RETURN(uvc_get_control_info(m_manager, m_device_id, &info), int);
	}

	/**
	 * UVC設定機能へ値を適用
	 * @param type
	 * @param value
	 * @return 0: 成功, 負: エラーコード
	 */
	int FlutterUVCHolder::set_control_value(const uint64_t &type, const int32_t &value) const
	{
		ENTER();
		RETURN(uvc_set_control_value(m_manager, m_device_id, type, value), int);
	}

	/**
	 * UVC設定機能の現在の値を取得
	 * @param type
	 * @param value
	 * @return 0: 成功, 負: エラーコード
	 */
	int FlutterUVCHolder::get_control_value(const uint64_t &type, int32_t &value) const
	{
		ENTER();
		RETURN(uvc_get_control_value(m_manager, m_device_id, type, &value), int);
	}

	/**
	 * UVC機器からの映像を受け取るためのSurface(ANativeWindow*)をセット
	 * @param preview_window
	 * @param mvp_matrix モデルビュー変換行列、要素数16以上, nullptrなら単位行列をセットする
	 * @return
	 */
	int FlutterUVCHolder::set_preview_surface(ANativeWindow *preview_window, const float *mvp_matrix)
	{
		ENTER();
		RETURN(uvc_set_surface(m_manager, m_device_id, preview_window, const_cast<float *>(mvp_matrix)), int);
	}

	/**
	 * 録画用のSurface(ANativeWindow*)をセット
	 * MediaCodecのencoderSurfaceを渡すことで録画を行う
	 * @param recording_window nullptrなら録画停止
	 * @return 0: 成功, 負: エラーコード
	 */
	int FlutterUVCHolder::set_recording_surface(ANativeWindow *recording_window)
	{
		ENTER();
		LOGD("set_recording_surface: window=%p, current=%p", recording_window, m_recording_window);

		// Stop existing recording thread if any
		if (m_recording_active)
		{
			LOGD("Stopping existing recording thread");
			m_recording_active = false;
			if (m_recording_thread && m_recording_thread->joinable())
			{
				m_recording_thread->join();
			}
			m_recording_thread.reset();
		}

		// Release old window reference
		if (m_recording_window)
		{
			ANativeWindow_release(m_recording_window);
			m_recording_window = nullptr;
		}

		// Set new window
		if (recording_window)
		{
			m_recording_window = recording_window;
			ANativeWindow_acquire(m_recording_window);

			// Configure window buffer format
			int32_t width = m_current_size.width > 0 ? m_current_size.width : 1280;
			int32_t height = m_current_size.height > 0 ? m_current_size.height : 720;
			ANativeWindow_setBuffersGeometry(m_recording_window, width, height, WINDOW_FORMAT_RGBA_8888);

			// Allocate frame buffer for RGBA conversion
			m_frame_buffer.resize(width * height * 4);

			// Start recording capture thread
			m_recording_active = true;
			m_recording_thread = std::make_unique<std::thread>(&FlutterUVCHolder::recording_capture_loop, this);
			LOGD("Recording thread started");
		}

		RETURN(0, int);
	}

	/**
	 * Recording frame capture loop - polls frames from UVC and renders to recording surface
	 */
	void FlutterUVCHolder::recording_capture_loop()
	{
		LOGD("recording_capture_loop started");

		// Allocate temporary buffers
		std::vector<uint8_t> yuv_buffer(m_current_size.width * m_current_size.height * 2);
		int64_t frame_count = 0;
		const int64_t frame_interval_ns = 1000000000LL / 30; // 30 FPS target
		auto last_frame_time = std::chrono::high_resolution_clock::now();

		while (m_recording_active && m_recording_window)
		{
			// Rate limit to avoid overwhelming the encoder
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_frame_time).count();
			if (elapsed < frame_interval_ns)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			last_frame_time = now;

			// Get frame from UVC camera
			uint32_t frame_type = RAW_FRAME_UNCOMPRESSED_RGBX; // Request RGBA directly if possible
			uint32_t width = m_current_size.width;
			uint32_t height = m_current_size.height;
			uint32_t data_len = m_frame_buffer.size();
			int64_t pts_us = 0;
			uint32_t flags = 0;

			int result = uvc_get_frame(m_manager, m_device_id,
									   &frame_type, &width, &height,
									   m_frame_buffer.data(), &data_len,
									   &pts_us, &flags);

			if (result != 0 || data_len == 0)
			{
				// No frame available, continue
				continue;
			}

			// Render to recording window
			ANativeWindow_Buffer buffer;
			if (ANativeWindow_lock(m_recording_window, &buffer, nullptr) == 0)
			{
				// Copy frame data to window buffer
				uint8_t *dst = static_cast<uint8_t *>(buffer.bits);
				const uint8_t *src = m_frame_buffer.data();

				int copy_width = std::min((int)width, buffer.width);
				int copy_height = std::min((int)height, buffer.height);
				int bytes_per_pixel = 4; // RGBA

				for (int y = 0; y < copy_height; y++)
				{
					memcpy(dst, src, copy_width * bytes_per_pixel);
					dst += buffer.stride * bytes_per_pixel;
					src += width * bytes_per_pixel;
				}

				ANativeWindow_unlockAndPost(m_recording_window);
				frame_count++;

				if (frame_count % 30 == 0)
				{
					LOGD("Recording frame %lld", (long long)frame_count);
				}
			}
		}

		LOGD("recording_capture_loop ended, total frames: %lld", (long long)frame_count);
	}

	/**
	 * モデルビュー変換行列を設定
	 * Surface(ANativeWindow)で映像を受け取るときのみ有効
	 * @param mvp_matrix モデルビュー変換行列、要素数16以上, nullptrなら単位行列をセットする
	 * @return
	 */
	int FlutterUVCHolder::set_mvp_matrix(const float *mvp_matrix)
	{
		ENTER();
		RETURN(uvc_set_mvp_matrix(m_manager, m_device_id, const_cast<float *>(mvp_matrix)), int);
	}

	int FlutterUVCHolder::set_video_size(
		const uvc_raw_frame_t &frame_type,
		const uint32_t &width, const uint32_t &height)
	{

		ENTER();
		auto r = uvc_resize(m_manager, m_device_id, frame_type, width, height);
		get_current_size();
		RETURN(r, int);
	}

	const uvc_video_size_t &FlutterUVCHolder::get_current_size()
	{
		ENTER();

		const auto r = uvc_get_current_size(m_manager, m_device_id, &m_current_size);
		if (UNLIKELY(r))
		{
			LOGD("uvc_get_current_size failed, err=%d", r);
		}

		RET(m_current_size);
	}

	int FlutterUVCHolder::get_supported_size(
		const int32_t &index, int32_t *num_supported,
		uvc_video_size_t *data) const
	{

		ENTER();
		const auto &supported = supported_size();
		const auto num = static_cast<int32_t>(supported.size());
		if (num_supported)
		{
			*num_supported = num;
		}
		int result = 1;
		if ((index >= 0) && (index < num))
		{
			if (data)
			{
				*data = supported[index];
			}
			result = 0;
		}

		RETURN(result, int);
	}

	int FlutterUVCHolder::start()
	{
		ENTER();
		RETURN(uvc_start(m_manager, m_device_id), int);
	}

	int FlutterUVCHolder::stop()
	{
		ENTER();
		RETURN(uvc_stop(m_manager, m_device_id), int);
	}

	/**
	 * 対応しているUVC設定機能一覧を更新する
	 */
	/*private*/
	void FlutterUVCHolder::update_supported_ctrls()
	{
		ENTER();

		const auto ctrls = uvc_get_ctrl_supports(m_manager, m_device_id);
		const auto procs = uvc_get_proc_supports(m_manager, m_device_id);

		m_supported_ctrls.clear();
		for (int i = 0; i < 32; i++)
		{
			const uint64_t type = 0x00000001llu << i;
			if ((ctrls & type) == type)
			{
				if (type == (CTRL_PAN_ABS & 0x00ffffff))
				{
					// PAN/TILTだけ特別扱いが必要(UVC規格場はPANとTILT2つ合わせて１つの設定項目)
					m_supported_ctrls.push_back(CTRL_PAN_ABS);
					m_supported_ctrls.push_back(CTRL_TILT_ABS);
				}
				else
				{
					m_supported_ctrls.push_back(type);
				}
			}
			if ((procs && type) == type)
			{
				m_supported_ctrls.push_back(type | PU_MASK);
			}
		}
		// 昇順にソート
		std::sort(m_supported_ctrls.begin(), m_supported_ctrls.end());

		EXIT();
	}

} // namespace serenegiant::flutter
