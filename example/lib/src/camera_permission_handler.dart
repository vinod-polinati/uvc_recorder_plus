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

import 'package:permission_handler/permission_handler.dart';

class CameraPermissionsHandler {
  // シングルトンパターンでアクセスできるようにする
  static final CameraPermissionsHandler _instance = CameraPermissionsHandler._internal();
  // ファクトリーコンストラクタ
  factory CameraPermissionsHandler() => _instance;

  // 内部使用のコンストラクタ
  CameraPermissionsHandler._internal();

  Future<bool> get isGranted async {
    final status = await Permission.camera.status;
    switch (status) {
      case PermissionStatus.granted:
      case PermissionStatus.limited:
        return true;
      case PermissionStatus.denied:
      case PermissionStatus.permanentlyDenied:
      case PermissionStatus.restricted:
        return false;
      default:
        return false;
    }
  }

  Future<PermissionStatus> request() async {
    return Permission.camera.request();
  }
}
