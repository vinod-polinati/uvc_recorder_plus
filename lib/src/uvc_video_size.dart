// Copyright (c) 2020-2025 saki t_saki@serenegiant.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

import 'package:uvc_recorder_plus/uvc_recorder_plus.dart';

/// 解像度設定
class VideoSize {
  final int frameType;

  /// フレームインデックス
  final int frameIndex;

  /// 映像幅[ピクセル数]
  final int width;

  /// 映像高さ[ピクセル数]
  final int height;

  /// フレームレートのタイプ
  /// 0: min/max/stepの3つのuint32_tで指定
  /// 正数: フレームインターバルデータの個数
  final int frameIntervalType;

  /// フレームインターバルデータ
  final List<int> frameIntervals;

  /// フレームインターバルデータの個数
  final int numFrameIntervals;

  /// フレームレート
  final List<double> fps;

  /// フレームレートの個数
  final int numFps;

  /// コンストラクタ
  VideoSize(
    this.frameType,
    this.frameIndex,
    this.width,
    this.height,
    this.frameIntervalType,
    this.frameIntervals,
    this.numFrameIntervals,
    this.fps,
    this.numFps,
  );

  /// 保持している解像度設定が有効かどうかを取得
  bool isValid() {
    return frameType != 0 && width != 0 && height != 0;
  }

  @override
  String toString() {
    return 'VideoSize{isValid:${isValid()}, frameType:$frameType/(${frameTypeString(frameType)}), frameIndex:$frameIndex, width:$width, height:$height, frameIntervalType:$frameIntervalType, frameIntervals:$frameIntervals, numFrameIntervals:$numFrameIntervals, fps:$fps, numFps:$numFps}';
  }

  String toShortString() {
    return '${width}x$height/${frameTypeString(frameType)}';
  }

  /// 無効な解像度設定値
  static final VideoSize INVALID = VideoSize(
    0,
    0,
    0,
    0,
    0,
    List.empty(),
    0,
    List.empty(),
    0,
  );

  /// フレームタイプ文字列を取得するためのヘルパー関数
  static String frameTypeString(int frameType) {
    switch (frameType) {
      case FRAME_TYPE_UNCOMPRESSED:
        return "UNCOMPRESSED:";
      case FRAME_TYPE_UNCOMPRESSED_YUYV:
        return "YUY2";
      case FRAME_TYPE_UNCOMPRESSED_UYVY:
        return "UYVY";
      case FRAME_TYPE_UNCOMPRESSED_GRAY8:
        return "GRAY8";
      case FRAME_TYPE_UNCOMPRESSED_BY8:
        return "BY8";
      case FRAME_TYPE_UNCOMPRESSED_NV21:
        return "NV21";
      case FRAME_TYPE_UNCOMPRESSED_YV12:
        return "YV12";
      case FRAME_TYPE_UNCOMPRESSED_I420:
        return "I420";
      case FRAME_TYPE_UNCOMPRESSED_Y16:
        return "Y16";
      case FRAME_TYPE_UNCOMPRESSED_RGBP:
        return "RGBP";
      case FRAME_TYPE_UNCOMPRESSED_NV12:
        return "NV12";
      case FRAME_TYPE_UNCOMPRESSED_YCbCr:
        return "YCbCr";
      case FRAME_TYPE_UNCOMPRESSED_RGB565:
        return "RGB565";
      case FRAME_TYPE_UNCOMPRESSED_RGB:
        return "RGB";
      case FRAME_TYPE_UNCOMPRESSED_BGR:
        return "BGR";
      case FRAME_TYPE_UNCOMPRESSED_RGBX:
        return "RGBX";
      case FRAME_TYPE_UNCOMPRESSED_444p:
        return "444P";
      case FRAME_TYPE_UNCOMPRESSED_444sp:
        return "444SP";
      case FRAME_TYPE_UNCOMPRESSED_422p:
        return "422P";
      case FRAME_TYPE_UNCOMPRESSED_422sp:
        return "422SP";
      case FRAME_TYPE_UNCOMPRESSED_440p:
        return "440P";
      case FRAME_TYPE_UNCOMPRESSED_440sp:
        return "440SP";
      case FRAME_TYPE_UNCOMPRESSED_411p:
        return "411P";
      case FRAME_TYPE_UNCOMPRESSED_411sp:
        return "411SP";
      case FRAME_TYPE_UNCOMPRESSED_YUV_ANY:
        return "YUVANY";
      case FRAME_TYPE_UNCOMPRESSED_XRGB:
        return "XRGB";
      case FRAME_TYPE_UNCOMPRESSED_XBGR:
        return "XBGR";
      case FRAME_TYPE_UNCOMPRESSED_BGRX:
        return "BGRX";
      case FRAME_TYPE_MJPEG:
        return "MJPEG";
      case FRAME_TYPE_FRAME_BASED:
        return "FRAME";
      case FRAME_TYPE_MPEG2TS:
        return "MPEG2TS";
      case FRAME_TYPE_DV:
        return "DV";
      case FRAME_TYPE_FRAME_H264:
        return "H264Frame";
      case FRAME_TYPE_FRAME_VP8:
        return "VP8Frame";
      case FRAME_TYPE_H264:
        return "H264";
      case FRAME_TYPE_H264_SIMULCAST:
        return "H264SIMUL";
      case FRAME_TYPE_VP8:
        return "VP8";
      case FRAME_TYPE_VP8_SIMULCAST:
        return "VP8SIMUL";
      case FRAME_TYPE_H265:
        return "H265";
      default:
        return "UNKNOWN";
    }
  }
}
