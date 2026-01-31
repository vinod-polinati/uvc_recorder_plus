/**
 * Copyright (c) 2020-2025 saki t_saki@serenegiant.com
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

#ifndef AANDUSB_FLUTTER_PLUGIN_H
#define AANDUSB_FLUTTER_PLUGIN_H

#ifdef __cplusplus
#include <cstdint>
#if defined(_WIN32)
#define EXTERN_C extern "C" __declspec(dllexport)
#else
#define EXTERN_C extern "C" __attribute__((visibility("default"))) __attribute__((used))
#endif
#else
#include <stdint.h>
#define EXTERN_C
#endif

/**
 * フレームインターバル/フレームレートの最大数
 * 規格上はこれより多い可能性があるけどとりあえず128に制限
 */
#define MAX_INTERVALS (128)

/**
 * UVC機器との接続状態
 * should match to device_state_t in aandusb_native.h
 */
typedef enum device_state {
	/**
	 * プラグインが初期化されていない
	 */
	UNINITIALIZED = -1,
	/**
	 * 指定した機器が接続されていない
	 */
	DISCONNECTED = 0,
	/**
	 * 指定した機器が接続されている(映像取得中ではない)
	 */
	CONNECTED = 1,
	/**
	 * 指定した機器が接続されており映像取得中
	 */
	STREAMING = 2,
} device_state_t;

// Camera Terminal DescriptorのbmControlsフィールドのビットマスク
#define	FLG_CTRL_SCANNING		(0x00000001)	// D0:  Scanning Mode
#define	FLG_CTRL_AE				(0x00000002)	// D1:  Auto-Exposure Mode
#define	FLG_CTRL_AE_PRIORITY	(0x00000004)	// D2:  Auto-Exposure Priority
#define	FLG_CTRL_AE_ABS			(0x00000008)	// D3:  Exposure Time (Absolute)
#define	FLG_CTRL_AE_REL			(0x00000010)	// D4:  Exposure Time (Relative)
#define FLG_CTRL_FOCUS_ABS    	(0x00000020)	// D5:  Focus (Absolute)
#define FLG_CTRL_FOCUS_REL		(0x00000040)	// D6:  Focus (Relative)
#define FLG_CTRL_IRIS_ABS		(0x00000080)	// D7:  Iris (Absolute)
#define	FLG_CTRL_IRIS_REL		(0x00000100)	// D8:  Iris (Relative)
#define	FLG_CTRL_ZOOM_ABS		(0x00000200)	// D9:  Zoom (Absolute)
#define FLG_CTRL_ZOOM_REL		(0x00000400)	// D10: Zoom (Relative)
#define	FLG_CTRL_PANTILT_ABS	(0x00000800)	// D11: PanTilt (Absolute)
#define	FLG_CTRL_PAN_ABS		(0x01000800)	// D11: PanTilt (Absolute)
#define	FLG_CTRL_TILT_ABS		(0x02000800)	// D11: PanTilt (Absolute)
#define FLG_CTRL_PANTILT_REL	(0x00001000)	// D12: PanTilt (Relative)
#define	FLG_CTRL_PAN_REL		(0x01001000)	// D12: PanTilt (Relative)
#define	FLG_CTRL_TILT_REL		(0x02001000)	// D12: PanTilt (Relative)
#define FLG_CTRL_ROLL_ABS		(0x00002000)	// D13: Roll (Absolute)
#define FLG_CTRL_ROLL_REL		(0x00004000)	// D14: Roll (Relative)
#define FLG_CTRL_D15			(0x00008000)	// D15: Reserved
#define FLG_CTRL_D16			(0x00010000)	// D16: Reserved
#define FLG_CTRL_FOCUS_AUTO		(0x00020000)	// D17: Focus, Auto
#define FLG_CTRL_PRIVACY		(0x00040000)	// D18: Privacy
#define FLG_CTRL_FOCUS_SIMPLE	(0x00080000)	// D19: Focus, Simple
#define FLG_CTRL_WINDOW			(0x00100000)	// D20: Window
#define FLG_CTRL_ROI			(0x00200000)	// D21: ROI
#define FLG_CTRL_D22			(0x00400000)	// D22: Reserved
#define FLG_CTRL_D23			(0x00800000)	// D23: Reserved

// Processing Unit DescriptorのbmControlsフィールドのビットマスク
#define FLG_PU_BRIGHTNESS		(0x80000001)	// D0: Brightness
#define FLG_PU_CONTRAST			(0x80000002)	// D1: Contrast
#define FLG_PU_HUE				(0x80000004)	// D2: Hue
#define	FLG_PU_SATURATION		(0x80000008)	// D3: Saturation
#define FLG_PU_SHARPNESS		(0x80000010)	// D4: Sharpness
#define FLG_PU_GAMMA			(0x80000020)	// D5: Gamma
#define	FLG_PU_WB_TEMP			(0x80000040)	// D6: White Balance Temperature
#define	FLG_PU_WB_COMPO			(0x80000080)	// D7: White Balance Component
#define	FLG_PU_BACKLIGHT		(0x80000100)	// D8: Backlight Compensation
#define FLG_PU_GAIN				(0x80000200)	// D9: Gain
#define FLG_PU_POWER_LF			(0x80000400)	// D10: Power Line Frequency
#define FLG_PU_HUE_AUTO			(0x80000800)	// D11: Hue, Auto
#define FLG_PU_WB_TEMP_AUTO		(0x80001000)	// D12: White Balance Temperature, Auto
#define FLG_PU_WB_COMPO_AUTO	(0x80002000)	// D13: White Balance Component, Auto
#define FLG_PU_DIGITAL_MULT		(0x80004000)	// D14: Digital Multiplier
#define FLG_PU_DIGITAL_LIMIT	(0x80008000)	// D15: Digital Multiplier Limit
#define FLG_PU_AVIDEO_STD		(0x80010000)	// D16: Analog Video Standard
#define FLG_PU_AVIDEO_LOCK		(0x80020000)	// D17: Analog Video Lock Status
#define FLG_PU_CONTRAST_AUTO	(0x80040000)	// D18: Contrast, Auto
#define FLG_PU_D19				(0x80080000)	// D19: Reserved
#define FLG_PU_D20				(0x80100000)	// D20: Reserved
#define FLG_PU_D21				(0x80200000)	// D21: Reserved
#define FLG_PU_D22				(0x80400000)	// D22: Reserved
#define FLG_PU_D23				(0x80800000)	// D23: Reserved

/** 不明なフレームフォーマット, 0x000000 */
#define FRAME_TYPE_UNKNOWN              (0x00000000),
/** 0x000005 不明な非圧縮フレームフォーマット */
#define FRAME_TYPE_UNCOMPRESSED         (0x00000005),
/** 0x010005, YUY2/YUYV/V422/YUV422, インターリーブ */
#define FRAME_TYPE_UNCOMPRESSED_YUYV    (0x00010005),
/** 0x020005, YUYVの順序違い、インターリーブ */
#define FRAME_TYPE_UNCOMPRESSED_UYVY    (0x00020005),
/** 0x030005, 8ビットグレースケール, Y800/Y8/I400 */
#define FRAME_TYPE_UNCOMPRESSED_GRAY8   (0x00030005),
/** 0x040005, 8ビットグレースケール, ベイヤー配列 */
#define FRAME_TYPE_UNCOMPRESSED_BY8     (0x00040005),
/** 0x050005, YVU420 SemiPlanar(y->vu), YVU420sp */
#define FRAME_TYPE_UNCOMPRESSED_NV21    (0x00050005),
/** 0x060005, YVU420 Planar(y->v->u), YVU420p, I420とU/Vが逆 */
#define FRAME_TYPE_UNCOMPRESSED_YV12    (0x00060005),
/** 0x070005, YUV420 Planar(y->u->v), YUV420p, YV12とU/Vが逆 */
#define FRAME_TYPE_UNCOMPRESSED_I420    (0x00070005),
/** 0x080005, 16ビットグレースケール */
#define FRAME_TYPE_UNCOMPRESSED_Y16     (0x00080005),
/** 0x090005, 18ビットインターリーブRGB(16ビット/5+6+5カラー), RGB565と並びが逆, RGB565 LE, BGR565 */
#define FRAME_TYPE_UNCOMPRESSED_RGBP    (0x00090005),
/** 0x0b0005, YUV420 SemiPlanar(y->uv) NV21とU/Vが逆 */
#define FRAME_TYPE_UNCOMPRESSED_NV12    (0x000b0005),
/** 0x0c0005, Y->Cb->Cr, インターリーブ(色空間を別にすればY->U->Vと同じ並び) */
#define FRAME_TYPE_UNCOMPRESSED_YCbCr   (0x000c0005),
/** 0x0d0005, 8ビットインターリーブRGB(16ビット/5+6+5カラー) */
#define FRAME_TYPE_UNCOMPRESSED_RGB565  (0x000d0005),
/** 0x0e0005, 8ビットインターリーブRGB(24ビットカラー), RGB24 */
#define FRAME_TYPE_UNCOMPRESSED_RGB     (0x000e0005),
/** 0x0f0005, 8ビットインターリーブBGR(24ビットカラー), BGR24 */
#define FRAME_TYPE_UNCOMPRESSED_BGR     (0x000f0005),
/** 0x100005, 8ビットインターリーブRGBX(32ビットカラー), RGBX32 */
#define FRAME_TYPE_UNCOMPRESSED_RGBX    (0x00100005),
/** 0x110005, YUV444 Planar(y->u->v), YUV444p */
#define FRAME_TYPE_UNCOMPRESSED_444p    (0x00110005),
/** 0x120005, YUV444 semi Planar(y->uv), YUV444sp */
#define FRAME_TYPE_UNCOMPRESSED_444sp   (0x00120005),
/** 0x130005, YUV422 Planar(y->u->v), YUV422p */
#define FRAME_TYPE_UNCOMPRESSED_422p    (0x00130005),
/** 0x140005, YUV422 SemiPlanar(y->uv), NV16/YUV422sp */
#define FRAME_TYPE_UNCOMPRESSED_422sp   (0x00140005),
/** 0x150005, YUV440 Planar(y->u->v), YUV440p */
#define FRAME_TYPE_UNCOMPRESSED_440p    (0x00150005),
/** 0x160005, YUV440 Planar(y->u->v), YUV440sp */
#define FRAME_TYPE_UNCOMPRESSED_440sp   (0x00160005),
/** 0x170005, YUV411 Planar(y->u->v), YUV411p */
#define FRAME_TYPE_UNCOMPRESSED_411p    (0x00170005),
/** 0x180005, YUV411 SemiPlanar(y->uv), YUV411sp */
#define FRAME_TYPE_UNCOMPRESSED_411sp   (0x00180005),
/** 0x190005, MJPEGからYUVプラナー系への変換先指定用、実際はYUVのプランナーフォーマットのいずれかになる */
#define FRAME_TYPE_UNCOMPRESSED_YUV_ANY (0x00190005),
/** 0x1a0005, 8ビットインターリーブXRGB(32ビットカラー), XRGB32 */
#define FRAME_TYPE_UNCOMPRESSED_XRGB    (0x001a0005),
/** 0x1b0005, 8ビットインターリーブXBGR(32ビットカラー), XBGR32 */
#define FRAME_TYPE_UNCOMPRESSED_XBGR    (0x001b0005),
/** 0x1c0005, 8ビットインターリーブBGRX(32ビットカラー), BGRX32 */
#define FRAME_TYPE_UNCOMPRESSED_BGRX    (0x001c0005),
/** mjpeg */
#define FRAME_TYPE_MJPEG                (0x00000007),
/** 不明なフレームベースのフレームフォーマット */
#define FRAME_TYPE_FRAME_BASED          (0x00000011),
/** MPEG2TS */
#define FRAME_TYPE_MPEG2TS              (0x00010011),
/** DV */
#define FRAME_TYPE_DV                   (0x00020011),
/** フレームベースのH.264 */
#define FRAME_TYPE_FRAME_H264           (0x00030011),
/** フレームベースのVP8 */
#define FRAME_TYPE_FRAME_VP8            (0x00040011),
/** H.264単独フレーム */
#define FRAME_TYPE_H264                 (0x00000014),
/** H.264 SIMULCAST */
#define FRAME_TYPE_H264_SIMULCAST       (0x00010014),
/** VP8単独フレーム */
#define FRAME_TYPE_VP8                  (0x00000017),
/** VP8 SIMULCAST */
#define FRAME_TYPE_VP8_SIMULCAST        (0x00010017),
/** H265単独フレーム */
#define FRAME_TYPE_H265                 (0x00000019),

/**
 * Flutterのc#側とUVCコントロール機能の設定値等をやりとりするための構造体定義
 * Flutterのc#側にも同じ構造体を定義する必要がある
 * should match to uvc_control_info_t in aandusb_native.h
 */
typedef struct flutter_control_info {
	uint64_t type;			// UVCコントロールの種類(CTRL_XXXまたはPU_XXX)
	int32_t initialized;	// 初期化済みかどうか
	int32_t has_min_max;	// 最大最小値を持つかどうか
	int32_t def;			// デフォルト値
	int32_t current;		// 現在値
	int32_t res;			// 分解能
	int32_t min;			// 最小値
	int32_t max;			// 最大値
} __attribute__((__packed__)) flutter_control_info_t;

/**
 * Flutterのc#側と映像サイズ設定をやりとりするための構造体定義
 * Flutterのc#側にも同じ構造体を定義する必要がある
 * should match to uvc_video_size_t in aandusb_native.h
 */
typedef struct flutter_video_size {
	uint32_t frame_type;
	/**
	 * フレームインデックス
	 */
	int32_t frame_index;
	/**
	 * 映像幅[ピクセル数]
	 */
	uint32_t width;
	/**
	 * 映像高さ[ピクセル数]
	 */
	uint32_t height;
	/**
	 * フレームレートのタイプ
	 * 0: min/max/stepの3つのuint32_tで指定
	 * 正数: フレームインターバルデータの個数
	 */
	int32_t frame_interval_type;
	/**
	 * フレームインターバルデータ
	 */
	uint32_t frame_intervals[MAX_INTERVALS];
	/**
	 * フレームインターバルデータの個数
	 */
	int32_t num_frame_intervals;
	/**
	 * フレームレート
	 */
	float fps[MAX_INTERVALS];
	/**
	 * フレームレートの個数
	 */
	int32_t num_fps;
} __attribute__((__packed__)) flutter_video_size_t;

/**
 * 接続しているUSB機器情報
 * should match to usb_device_info_t in aandusb_native.h
 */
typedef struct flutter_device_info {
	uint32_t vendor_id;
	uint32_t product_id;
	uint8_t device_class;
	uint8_t device_subclass;
	uint8_t device_protocol;
	uint8_t reserved1;
	uint8_t name[128];
	uint8_t manufacturer_name[128];
	uint8_t product_name[128];
	uint8_t serial[128];
} __attribute__((__packed__)) flutter_device_info_t;

//--------------------------------------------------------------------------------
// DartのFlutterプラグイン部分から呼ばれる関数

EXTERN_C
int64_t initialize_dart_api(void *data);

EXTERN_C
void set_dart_api_message_port(int64_t port);

EXTERN_C
int32_t get_state(int32_t device_id);

EXTERN_C
int32_t  get_device_info(int32_t device_id, flutter_device_info_t *info_out);

EXTERN_C
int64_t start(int32_t device_id);

EXTERN_C
int32_t stop(int32_t device_id);

EXTERN_C
int set_video_size(
	int32_t device_id,
	uint32_t type,
	uint32_t width, uint32_t height);

EXTERN_C
int get_current_size(
	int32_t device_id,
	flutter_video_size_t *data);

/**
 * コントロール機能でサポートしている機能を取得
 * @param device_id
 * @return
 */
EXTERN_C
uint64_t get_ctrl_supports(int32_t device_id);

/**
 * プロセッシングユニットでサポートしている機能を取得
 * @param device_id
 * @return
 */
EXTERN_C
uint64_t get_proc_supports(int32_t device_id);

/**
 * 指定した機能の設定情報を取得
 * @param device_id
 * @param value
 * @return
 */
EXTERN_C
int32_t get_ctrl_info(int32_t device_id, flutter_control_info_t *value);

/**
 * 指定した機能の設定値を適用
 * @param device_id
 * @param type
 * @param value
 * @return
 */
EXTERN_C
int32_t set_ctrl_value(int32_t device_id, uint64_t type, int32_t value);

/**
 * 指定した機能の設定値を取得
 * @param device_id
 * @param type
 * @param value
 * @return
 */
EXTERN_C
int32_t get_ctrl_value(int32_t device_id, uint64_t type, int32_t *value);

/**
 * native側でUVC映像サイズ設定へアクセスするときのヘルパー関数
 * 主にUnityやFlutterからのアクセスを想定
 * @param device_id
 * @param index 映像サイズ設定のインデックス
 * @param num_supported 対応している映像サイズ設定の数を入れるuint32_tへのポインタ
 * @param data 映像サイズ設定を書き込むためのunity_video_size_t構造体へのポインタ
 * @return 0: 成功, 負: エラーコード
 */
EXTERN_C
int32_t get_supported_size(
	int32_t device_id,
	int32_t index, int32_t *num_supported, flutter_video_size_t *data);

/**
 * 映像取得用のsurfaceをセットする
 * @param device_id UVC機器の識別子
 * @param tex_id   テクスチャID
 * @param jsurface Java側のSurfaceオブジェクト
 */
EXTERN_C
int32_t set_preview_surface(
	int32_t device_id,	// jint
	int64_t tex_id,		// jlong
	void *jsurface);	// jobject jsurface

#endif //AANDUSB_FLUTTER_PLUGIN_H
