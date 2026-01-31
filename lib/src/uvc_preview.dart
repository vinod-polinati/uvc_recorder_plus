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

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:logger/logger.dart';

import 'package:uvc_recorder_plus/uvc_manager.dart';

//--------------------------------------------------------------------------------
// 定数定義
const bool _debug = false;

//--------------------------------------------------------------------------------
final _logger = Logger(
  printer: PrettyPrinter(
    methodCount: 0,
    errorMethodCount: 3,
    dateTimeFormat: DateTimeFormat.none,
    excludeBox: {
      Level.trace: true,
      Level.debug: true,
      Level.info: true,
      Level.warning: false,
      Level.error: false,
    },
  ),
);

//--------------------------------------------------------------------------------
/// UVC機器からの映像を表示するウィジェット
class UVCVideoView extends StatefulWidget {
  final int deviceId;

  /// デフォルトのフレームタイプ
  final int frameType;

  /// デフォルトの解像度(幅)
  final int videoWidth;

  /// デフォルトの解像度(高さ)
  final int videoHeight;

  /// 映像取得待機中のメッセージウィジェット
  final Widget? waitingMessage;

  /// 解像度選択やUVC機器コントロール用のウィジェット
  final Widget? controlView;

  const UVCVideoView({
    super.key,
    required this.deviceId,
    this.frameType = 7,
    this.videoWidth = 640,
    this.videoHeight = 480,
    this.waitingMessage,
    this.controlView,
  });

  @override
  State<StatefulWidget> createState() => UVCVideoViewState();
}

/// State for UVCVideoView
/// 親WidgetからsetSizeを呼べるようにpublicにする
class UVCVideoViewState extends State<UVCVideoView>
    with WidgetsBindingObserver {
  /// UVC機器アクセス用
  late UVCControllerInterface _controller;

  /// 現在の解像度設定
  VideoSize _currentSize = VideoSize(
    7,
    0,
    640,
    480,
    0,
    List.empty(),
    0,
    List.empty(),
    0,
  );

  /// UVC機器からの映像を表示するときに使うテクスチャID
  int _textureId = -1;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
    _currentSize = VideoSize(
      widget.frameType,
      0,
      widget.videoWidth,
      widget.videoHeight,
      0,
      List.empty(),
      0,
      List.empty(),
      0,
    );
    _controller = UVCManagerPlatform.instance.getController(widget.deviceId);
    if (_debug)
      _logger.d("_UVCVideoViewState#initState:controller=$_controller");
    _initTexture();
  }

  @override
  Future<void> dispose() async {
    if (_debug) _logger.d("_UVCVideoViewState#dispose:");
    WidgetsBinding.instance.removeObserver(this);
    await _controller.stop();
    await _controller.releaseTexture();
    super.dispose();
  }

  @override
  Future<void> didChangeAppLifecycleState(AppLifecycleState state) async {
    if (_debug)
      _logger.d("_UVCVideoViewState#didChangeAppLifecycleState:state=$state");
    // アプリがバックグラウンドになるときは
    //     inactive -> hidden -> paused -> detached
    // resumeが来る時は
    //     inactive -> resume
    switch (state) {
      case AppLifecycleState.resumed:
        final sz = await _controller.getCurrentSize();
        final textureId = await _controller.createTexture(sz.width, sz.height);
        if (_debug) _logger.d("_UVCVideoViewState#textureId=$textureId,sz=$sz");
        _controller.start();
        setState(() {
          _textureId = textureId;
          _currentSize = sz;
        });
        break;
      case AppLifecycleState.inactive:
        break;
      case AppLifecycleState.hidden:
        break;
      case AppLifecycleState.paused:
        await _controller.stop();
        await _controller.releaseTexture();
        setState(() {
          _textureId = -1;
        });
        break;
      case AppLifecycleState.detached:
        break;
    }
  }

  @override
  Widget build(BuildContext context) {
    if (_debug) _logger.d("_UVCVideoViewState#build:");
    return Container(
      color: const Color.fromARGB(255, 0, 0, 0),
      alignment: Alignment.center,
      child:
          (_textureId < 0)
              ? widget.waitingMessage
              : (widget.controlView != null)
              ? Stack(
                // 解像度選択・UVC機器コントロール機能用ウィジェットを表示するとき
                fit: StackFit.expand,
                children: [
                  _CropCenterWidget(
                    size: _currentSize,
                    child: Texture(textureId: _textureId),
                  ),
                  widget.controlView!,
                ],
              )
              : _CropCenterWidget(
                size: _currentSize,
                child: Texture(textureId: _textureId),
              ),
    );
  }

  /// 解像度変更処理
  void setSize(VideoSize size) {
    if (_debug) _logger.d("_UVCVideoViewState#_setSize:$size");
    _controller
        .setSize(size.frameType, size.width, size.height)
        .then(
          (sz) => setState(() {
            _currentSize = sz;
          }),
        );
  }

  /// UVC機器映像取得用のテクスチャ/surfaceを初期化して映像取得を開始する
  Future<Null> _initTexture() async {
    if (_debug) _logger.d("_UVCVideoViewState#initTexture:");
    final sz = await _controller.setSize(
      _currentSize.frameType,
      _currentSize.width,
      _currentSize.height,
    );
    if (!sz.isValid()) {
      _logger.w("failed to set video size,sz=$sz/$_currentSize");
      return;
    }
    final textureId = await _controller.createTexture(sz.width, sz.height);
    if (_debug) _logger.d("_UVCVideoViewState#textureId=$textureId,sz=$sz");
    _controller.start();
    setState(() {
      _textureId = textureId;
      _currentSize = sz;
    });
  }
}

/// childを親Widgetの中央にクロップセンター表示するWidget
class _CropCenterWidget extends StatefulWidget {
  final VideoSize size;
  final Widget? child;

  const _CropCenterWidget({required this.size, this.child});

  @override
  State<StatefulWidget> createState() => _CropCenterWidgetState();
}

class _CropCenterWidgetState extends State<_CropCenterWidget> {
  @override
  Widget build(BuildContext context) {
    var width = widget.size.width.toDouble();
    if (width == 0) {
      width = 640.0;
    }
    var height = widget.size.height.toDouble();
    if (height == 0) {
      height = 480.0;
    }
    if (_debug) _logger.d("_CropCenterWidget:(${width}x$height)");
    return ClipRect(
      child: OverflowBox(
        maxWidth: double.infinity,
        maxHeight: double.infinity,
        alignment: Alignment.center,
        child: FittedBox(
          fit: BoxFit.fill,
          alignment: Alignment.center,
          child: SizedBox(width: width, height: height, child: widget.child),
        ),
      ),
    );
  }
}
