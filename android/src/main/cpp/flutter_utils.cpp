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

#define LOG_TAG "FlutterUtils"

#if 1	// デバッグ情報を出さない時は1
	#ifndef LOG_NDEBUG
		#define	LOG_NDEBUG		// LOGV/LOGD/MARKを出力しない時
	#endif
	#undef USE_LOGALL			// 指定したLOGxだけを出力
	#ifndef LOG_NDEBUG
		#define	LOG_NDEBUG		// LOGV/LOGD/MARKを出力しない時
	#endif
#else
	#define USE_LOGALL
	#define USE_LOGD
	#undef LOG_NDEBUG
	#undef NDEBUG
#endif

// 標準ライブラリ
#include <cstdarg>
// dart
#include "dartAPIDL/dart_api_dl.h"
#include "dartAPIDL/dart_native_api.h"
// aandusb
#include "utilbase.h"
// plugin
#include "flutter_utils.h"

static int64_t dart_api_message_port = -1;

DART_EXPORT
int64_t initialize_dart_api(void *data) {
	ENTER();
	LOGD("Dart_InitializeApiDL");
	RETURN(Dart_InitializeApiDL(data), int64_t);
}

DART_EXPORT
void set_dart_api_message_port(int64_t port) {
	ENTER();
	LOGD("port=%" FMT_INT64_T, port);
	dart_api_message_port = port;
	EXIT();
}

namespace serenegiant::flutter {

/**
 * 指定したデータをDartのReceiverPortへ送信する
 * @tparam char_t
 * @tparam Args
 * @param action 送信するデータのアクション
 * @param args...
 * @return
 */
template <typename Char, typename ...Args>
int send_msg_to_flutter(const Char *action, Args * ...args) {
	ENTER();

	if (dart_api_message_port == -1) {
		RETURN(-29, int);
	}

	Dart_CObject action_obj = {
		.type = Dart_CObject_kString,
		.value {
			.as_string = (char *)action,
		}
	};
	Dart_CObject *array[] = {&action_obj, args...};
	Dart_CObject obj = {
		.type = Dart_CObject_kArray,
		.value {
			.as_array {
				.length = (sizeof...(Args) + 1),
				.values = array,
			}
		}
	};

	LOGD("call Dart_PostCObject_DL");
	Dart_PostCObject_DL(dart_api_message_port, &obj);

	RETURN(0, int);
}

/**
 * USB機器の接続状態変化イベントをnative portを使ってDartへ送信する
 * action="on_device_changed"
 * @param device_id
 * @param attached
 * @return
 */
int send_on_device_changed(const int32_t &device_id, const bool &attached) {
	ENTER();

	if (dart_api_message_port == -1) {
		RETURN(-29, int);
	}
	Dart_CObject arg1 = {
		.type = Dart_CObject_kInt32,
		.value {
			.as_int32 = device_id
		}
	};
	Dart_CObject arg2 = {
		.type = Dart_CObject_kBool,
		.value {
			.as_int32 = attached
		}
	};
	send_msg_to_flutter("on_device_changed", &arg1, &arg2);

	RETURN(0, int);
}

}	// namespace serenegiant::flutter
