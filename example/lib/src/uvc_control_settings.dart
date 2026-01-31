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

import 'package:flutter/material.dart';

import 'package:uvc_recorder_plus/uvc_recorder_plus.dart';

class ControlSettings {
  /// UVC機器コントロール機能の種類
  final int type;
  final String title;
  final IconData? icon;

  /// 自動設定に対応しているときに自動設置の有効無効を切り替えるためのUVC機器コントロール機能の種類
  final int autoType;

  /// セレクタタイプの時に使う設定値
  /// dartはMapがordered mapらしいのでList&lt;int&gt;とList&lt;String&gtを別々に持つ代わりにMapで保持する
  final Map<int, String> selectors;

  ControlSettings({
    required this.type,
    required this.title,
    required this.icon,
    this.autoType = 0,
    this.selectors = const {},
  });

  ControlSettings copyWith(Map<int, String> selectors) {
    return ControlSettings(
      type: type,
      title: title,
      icon: icon,
      autoType: autoType,
      selectors: selectors,
    );
  }

  /// 自動設定が存在しているかどうか
  bool get hasAuto => autoType != 0;

  @override
  String toString() {
    return 'ControlSettings{type: $type, title: $title, icon: $icon, autoType: $autoType, selectors: $selectors}';
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is ControlSettings &&
          runtimeType == other.runtimeType &&
          type == other.type &&
          title == other.title &&
          icon == other.icon &&
          autoType == other.autoType &&
          selectors == other.selectors;

  @override
  int get hashCode =>
      type.hashCode ^
      title.hashCode ^
      icon.hashCode ^
      autoType.hashCode ^
      selectors.hashCode;

  static ControlSettings? get(int type) {
    return supportedControlSettings.containsKey(type)
        ? supportedControlSettings[type]
        : null;
  }

  /// XXX AUTO系のうち自動露出設定以外はON/OFFで対応する手動設定が存在しておりそのエントリーのautoTypeで指定するので個別には使わない
  /// XXX 相対設定系は変更速度と方向を設定しないといけないのでサンプルアプリの単純なUIでは実装が難しいので使わない
  static final supportedControlSettings = <int, ControlSettings>{
    FLG_CTRL_SCANNING: ControlSettings(
      // 0x00000001;	// D0:  Scanning Mode
      type: FLG_CTRL_SCANNING,
      title: "CTRL_SCANNING",
      icon: Icons.restore,
    ),
    FLG_CTRL_AE: ControlSettings(
      // 0x00000002;	// D1:  Auto-Exposure Mode
      type: FLG_CTRL_AE,
      title: "CTRL_AE_MODE",
      icon: Icons.restore,
      selectors: {
        0x01: "MANUAL",
        0x02: "AUTO",
        0x04: "SHUTTER_PRIORITY",
        0x08: "APERTURE_PRIORITY",
      },
    ),
    FLG_CTRL_AE_PRIORITY: ControlSettings(
      // 0x00000004;	// D2:  Auto-Exposure Priority
      type: FLG_CTRL_AE_PRIORITY,
      title: "CTRL_AE_PRIORITY",
      icon: Icons.restore,
      selectors: {0: "CONSTANT", 1: "VARY"},
    ),
    FLG_CTRL_AE_ABS: ControlSettings(
      // 0x00000008;	// D3:  Exposure Time (Absolute)
      type: FLG_CTRL_AE_ABS,
      title: "CTRL_EXP_ABS",
      icon: Icons.restore,
    ),
    // FLG_CTRL_AE_REL: ControlSettings(      // 0x00000010;	// D4:  Exposure Time (Relative)
    //   type: FLG_CTRL_AE_REL,
    //   title: "CTRL_EXP_REL",
    //   icon: Icons.restore,
    // ),
    FLG_CTRL_FOCUS_ABS: ControlSettings(
      // 0x00000020;	// D5:  Focus (Absolute)
      type: FLG_CTRL_FOCUS_ABS,
      title: "CTRL_FOCUS_ABS",
      icon: Icons.restore,
      autoType: FLG_CTRL_FOCUS_AUTO,
    ),
    // FLG_CTRL_FOCUS_REL: ControlSettings( // 0x00000040;	// D6:  Focus (Relative)
    //     type: FLG_CTRL_FOCUS_REL,
    //     title: "CTRL_FOCUS_REL",
    //     icon: Icons.restore,
    //     autoType: FLG_CTRL_FOCUS_AUTO
    // ),
    FLG_CTRL_IRIS_ABS: ControlSettings(
      // 0x00000080;	// D7:  Iris (Absolute)
      type: FLG_CTRL_IRIS_ABS,
      title: "CTRL_IRIS_ABS",
      icon: Icons.restore,
    ),
    // FLG_CTRL_IRIS_REL: ControlSettings( // 0x00000100;	// D8:  Iris (Relative)
    //   type: FLG_CTRL_IRIS_REL,
    //   title: "CTRL_IRIS_REL",
    //   icon: Icons.restore,
    // ),
    FLG_CTRL_ZOOM_ABS: ControlSettings(
      // 0x00000200;	// D9:  Zoom (Absolute)
      type: FLG_CTRL_ZOOM_ABS,
      title: "CTRL_ZOOM_ABS",
      icon: Icons.restore,
    ),
    // FLG_CTRL_ZOOM_REL: ControlSettings(    // 0x00000400;	// D10: Zoom (Relative)
    //   type: FLG_CTRL_ZOOM_REL,
    //   title: "CTRL_ZOOM_REL",
    //   icon: Icons.restore,
    // ),
    FLG_CTRL_PAN_ABS: ControlSettings(
      // 0x00000800;	// D11: PanTilt (Absolute)
      type: FLG_CTRL_PAN_ABS,
      title: "CTRL_PAN_ABS",
      icon: Icons.restore,
    ),
    FLG_CTRL_TILT_ABS: ControlSettings(
      // 0x00000800;	// D11: PanTilt (Absolute)
      type: FLG_CTRL_TILT_ABS,
      title: "CTRL_TILT_ABS",
      icon: Icons.restore,
    ),
    // FLG_CTRL_PAN_REL: ControlSettings( // 0x00001000;	// D12: PanTilt (Relative)
    //   type: FLG_CTRL_PAN_REL,
    //   title: "CTRL_PAN_REL",
    //   icon: Icons.restore,
    // ),
    // FLG_CTRL_TILT_REL: ControlSettings( // 0x00001000;	// D12: PanTilt (Relative)
    //   type: FLG_CTRL_TILT_REL,
    //   title: "CTRL_TILT_REL",
    //   icon: Icons.restore,
    // ),
    FLG_CTRL_ROLL_ABS: ControlSettings(
      // 0x00002000;	// D13: Roll (Absolute)
      type: FLG_CTRL_ROLL_ABS,
      title: "CTRL_ROLL_ABS",
      icon: Icons.restore,
    ),
    // FLG_CTRL_ROLL_REL: ControlSettings(    // 0x00004000;	// D14: Roll (Relative)
    //   type: FLG_CTRL_ROLL_REL,
    //   title: "CTRL_ROLL_REL",
    //   icon: Icons.restore,
    // ),
    // ControlSettings(  // 0x00020000;	// D17: Focus, Auto
    //   type: FLG_CTRL_FOCUS_AUTO,
    //   title: "CTRL_FOCUS_AUTO",
    //   icon: Icons.restore,
    // ),
    FLG_CTRL_PRIVACY: ControlSettings(
      // 0x00040000;	// D18: Privacy
      type: FLG_CTRL_PRIVACY,
      title: "CTRL_PRIVACY",
      icon: Icons.restore,
    ),
    // FLG_CTRL_FOCUS_SIMPLE: ControlSettings(// 0x00080000;	// D19: Focus, Simple
    //   type: FLG_CTRL_FOCUS_SIMPLE,
    //   title: "CTRL_FOCUS_SIMPLE",
    //   icon: Icons.restore,
    // ),
    // FLG_CTRL_WINDOW: ControlSettings(      // 0x00100000;	// D20: Window
    //   type: FLG_CTRL_WINDOW,
    //   title: "CTRL_WINDOW",
    //   icon: Icons.restore,
    // ),
    FLG_PU_BRIGHTNESS: ControlSettings(
      // 0x80000001;	// D0: Brightness
      type: FLG_PU_BRIGHTNESS,
      title: "PU_BRIGHTNESS",
      icon: Icons.restore,
    ),
    FLG_PU_CONTRAST: ControlSettings(
      // 0x80000002;	// D1: Contrast
      type: FLG_PU_CONTRAST,
      title: "PU_CONTRAST",
      icon: Icons.restore,
      autoType: FLG_PU_CONTRAST_AUTO,
    ),
    FLG_PU_HUE: ControlSettings(
      // 0x80000004;	// D2: Hue
      type: FLG_PU_HUE,
      title: "PU_HUE, R.drawable.ic_hue",
      icon: Icons.restore,
      autoType: FLG_PU_HUE_AUTO,
    ),
    FLG_PU_SATURATION: ControlSettings(
      // 0x80000008;	// D3: Saturation
      type: FLG_PU_SATURATION,
      title: "PU_SATURATION",
      icon: Icons.restore,
    ),
    FLG_PU_SHARPNESS: ControlSettings(
      // 0x80000010;	// D4: Sharpness
      type: FLG_PU_SHARPNESS,
      title: "PU_SHARPNESS",
      icon: Icons.restore,
    ),
    FLG_PU_GAMMA: ControlSettings(
      // 0x80000020;	// D5: Gamma
      type: FLG_PU_GAMMA,
      title: "PU_GAMMA",
      icon: Icons.restore,
    ),
    FLG_PU_WB_TEMP: ControlSettings(
      // 0x80000040;	// D6: White Balance Temperature
      type: FLG_PU_WB_TEMP,
      title: "PU_WB_TEMP",
      icon: Icons.restore,
      autoType: FLG_PU_WB_TEMP_AUTO,
    ),
    FLG_PU_WB_COMPO: ControlSettings(
      // 0x80000080;	// D7: White Balance Component
      type: FLG_PU_WB_COMPO,
      title: "PU_WB_COMPO",
      icon: Icons.restore,
      autoType: FLG_PU_WB_COMPO_AUTO,
    ),
    FLG_PU_BACKLIGHT: ControlSettings(
      // 0x80000100;	// D8: Backlight Compensation
      type: FLG_PU_BACKLIGHT,
      title: "PU_BACKLIGHT",
      icon: Icons.restore,
    ),
    FLG_PU_GAIN: ControlSettings(
      // 0x80000200;	// D9: Gain
      type: FLG_PU_GAIN,
      title: "PU_GAIN",
      icon: Icons.restore,
    ),
    FLG_PU_POWER_LF: ControlSettings(
      // 0x80000400;	// D10: Power Line Frequency
      type: FLG_PU_POWER_LF,
      title: "PU_POWER_LF",
      icon: Icons.restore,
      selectors: {0: "DISABLE", 1: "50HZ", 2: "60HZ", 3: "AUTO"},
    ),
    // FLG_PU_HUE_AUTO: ControlSettings(      // 0x80000800;	// D11: Hue, Auto
    //   type: FLG_PU_HUE_AUTO,
    //   title: "PU_HUE_AUTO",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_WB_TEMP_AUTO: ControlSettings(  // 0x80001000;	// D12: White Balance Temperature, Auto
    //   type: FLG_PU_WB_TEMP_AUTO,
    //   title: "PU_WB_TEMP_AUTO",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_WB_COMPO_AUTO: ControlSettings( // 0x80002000;	// D13: White Balance Component, Auto
    //   type: FLG_PU_WB_COMPO_AUTO,
    //   title: "PU_WB_COMPO_AUTO",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_DIGITAL_MULT: ControlSettings(  // 0x80004000;	// D14: Digital Multiplier
    //   type: FLG_PU_DIGITAL_MULT,
    //   title: "PU_DIGITAL_MULT",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_DIGITAL_LIMIT: ControlSettings( // 0x80008000;	// D15: Digital Multiplier Limit
    //   type: FLG_PU_DIGITAL_LIMIT,
    //   title: "PU_DIGITAL_LIMIT",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_AVIDEO_STD: ControlSettings(    // 0x80010000;	// D16: Analog Video Standard
    //   type: FLG_PU_AVIDEO_STD,
    //   title: "PU_AVIDEO_STD",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_AVIDEO_LOCK: ControlSettings(   // 0x80020000;	// D17: Analog Video Lock Status
    //   type: FLG_PU_AVIDEO_LOCK,
    //   title: "PU_AVIDEO_LOCK",
    //   icon: Icons.restore,
    // ),
    // FLG_PU_CONTRAST_AUTO: ControlSettings( // 0x80040000;	// D18: Contrast, Auto
    //   type: FLG_PU_CONTRAST_AUTO,
    //   title: "PU_CONTRAST_AUTO",
    //   icon: Icons.restore,
    // ),
  };
}
