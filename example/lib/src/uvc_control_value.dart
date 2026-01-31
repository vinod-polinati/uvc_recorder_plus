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
import 'uvc_control_settings.dart';

/// UVC機器コントロール処理
class ControlValue {
  /// 機器識別ID
  final int deviceId;

  /// UVC機器から取得したUVCコントロール機能の設定値
  final ControlInfo info;

  /// UVCコントロール機能の設定パラメータ
  final ControlSettings _settings;

  /// 自動設定が利用可能かどうか
  bool _canAuto = false;

  /// 自動設定かどうか
  bool _auto = false;

  /// この設定項目が有効かどうか
  bool _enabled = true;

  /// セレクタタイプかどうか
  bool _selectable = false;

  ControlValue(this.deviceId, this.info, this._settings) {
    _canAuto = _settings.hasAuto;
    _selectable = _settings.selectors.isNotEmpty == true;
  }

  ControlValue copyWith(int currentValue) {
    final result = ControlValue(deviceId, info, _settings);
    result.info.current = currentValue;
    return result;
  }

  /// この設定項目が有効かどうか
  bool get isEnabled => _enabled;
  set isEnabled(bool v) {
    _enabled = v;
  }

  /// この設定項目に対応する自動設定項目が存在するかどうか
  bool get hasAuto => _canAuto && _settings.hasAuto;
  set hasAuto(bool v) {
    _canAuto = v;
    if (!v) {
      isAuto = false;
    }
  }

  /// 現在自動設定になっているかどうか
  bool get isAuto => hasAuto && _auto;
  set isAuto(bool v) {
    _auto = hasAuto && v;
  }

  /// UVCコントロール機能の種類
  int get type => info.type;

  /// 自動設定に対応しているときに自動設置の有効無効を切り替えるためのUVC機器コントロール機能の種類
  int get autoType => _settings.autoType;

  /// 現在の設定値
  int get current => info.current;
  set current(v) {
    info.current = v;
  }

  /// 設定値の最小値(セレクタタイプの場合は無効)
  int get min => info.min;

  /// 設定値の最大値(セレクタタイプの場合は無効)
  int get max => info.max;

  /// 設定のデフォルト値
  int get def => info.def;

  /// 設定値のステップ(セレクタタイプの場合は無効)
  int get res => info.res;

  /// 設定項目タイトル文字列
  String get title => ControlInfo.typeString(info.type);

  /// 設定項目用アイコン
  IconData? get icon => _settings.icon;

  /// 現在の設定値ラベル
  String get label => _getLabel();

  String _getLabel() {
    if (_selectable) {
      return _settings.selectors[current] ?? current.toString();
    } else {
      return current.toString();
    }
  }

  /// 設定用スライダーの最小値
  double get minProgress => 0.0;

  /// 設定用スラーだーの最大値
  double get maxProgress =>
      _selectable
          ? (_settings.selectors.length - 1).toDouble()
          : (info.max - info.min).abs().toDouble();

  /// 現在の設定用スライダーの値
  double get progress => _getProgress();
  set progress(double v) {
    if (_selectable) {
      // セレクタタイプの時にprogressへセットされるのは設定値のインデックス
      final ix = v.toInt();
      for (final (index, key) in _settings.selectors.keys.indexed) {
        if (index == ix) {
          info.current = key;
          break;
        }
      }
    } else {
      info.current = (v + min).toInt();
    }
  }

  double _getProgress() {
    if (_selectable) {
      final cur = current;
      for (final (index, key) in _settings.selectors.keys.indexed) {
        if (key == cur) {
          return index.toDouble();
        }
      }
      return 0.0;
    } else {
      var result = (current - min).toDouble();
      if (result < 0) result = 0;
      return result;
    }
  }

  @override
  String toString() {
    return 'ControlValue{deviceId:$deviceId, _info:$info, _settings:$_settings, _canAuto:$_canAuto, _auto:$_auto, _enabled:$_enabled, _selectable:$_selectable}';
  }
}
