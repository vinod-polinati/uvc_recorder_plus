import 'dart:io';
import 'package:flutter/services.dart';
import 'package:path_provider/path_provider.dart';

/// Service for recording video from UVC camera using the native frame renderer.
///
/// Uses the uvc_manager plugin with recording support via the
/// Native C++ frame capture mechanism.
class UvcRecorder {
  // Use the same method channel as uvc_manager
  static const MethodChannel _channel = MethodChannel(
    'com.serenegiant.flutter/aandusb_method',
  );

  static bool _isRecording = false;
  static String? _currentRecordingPath;
  static int? _currentDeviceId;

  /// Check if currently recording
  static bool get isRecording => _isRecording;

  /// Get current recording file path (if recording)
  static String? get currentRecordingPath => _currentRecordingPath;

  /// Start recording video from UVC camera.
  ///
  /// [deviceId] is the UVC device ID (required).
  /// [width] and [height] specify the video resolution (default 1280x720).
  /// [bitrate] specifies encoding bitrate in bps (default 4 Mbps).
  /// [customPath] optional custom path for the output file.
  ///
  /// Returns the output file path where the video will be saved.
  /// Throws [PlatformException] if recording fails to start.
  static Future<String> startRecording({
    required int deviceId,
    int width = 1280,
    int height = 720,
    int bitrate = 4000000,
    String? customPath,
  }) async {
    if (_isRecording) {
      throw StateError('Already recording');
    }

    // Generate output path if not provided
    final String outputPath;
    if (customPath != null) {
      outputPath = customPath;
    } else {
      final directory = await getTemporaryDirectory();
      final timestamp = DateTime.now().millisecondsSinceEpoch;
      outputPath = '${directory.path}/uvc_recording_$timestamp.mp4';
    }

    // Ensure parent directory exists
    final file = File(outputPath);
    await file.parent.create(recursive: true);

    try {
      await _channel.invokeMethod('startRecording', {
        'deviceId': deviceId,
        'path': outputPath,
        'width': width,
        'height': height,
        'bitrate': bitrate,
      });

      _isRecording = true;
      _currentRecordingPath = outputPath;
      _currentDeviceId = deviceId;

      return outputPath;
    } catch (e) {
      _isRecording = false;
      _currentRecordingPath = null;
      _currentDeviceId = null;
      rethrow;
    }
  }

  /// Stop recording and finalize the video file.
  ///
  /// Returns the path to the recorded video file, or null if not recording.
  /// Throws [PlatformException] if stopping fails.
  static Future<String?> stopRecording() async {
    if (!_isRecording || _currentDeviceId == null) {
      return null;
    }

    try {
      final String? path = await _channel.invokeMethod('stopRecording', {
        'deviceId': _currentDeviceId,
      });
      _isRecording = false;
      _currentRecordingPath = null;
      _currentDeviceId = null;
      return path;
    } catch (e) {
      _isRecording = false;
      _currentRecordingPath = null;
      _currentDeviceId = null;
      rethrow;
    }
  }

  /// Query the native recording state.
  ///
  /// This is useful to sync state after app restart.
  static Future<bool> queryIsRecording(int deviceId) async {
    try {
      final bool? result = await _channel.invokeMethod('isRecording', {
        'deviceId': deviceId,
      });
      _isRecording = result ?? false;
      return _isRecording;
    } catch (e) {
      return false;
    }
  }

  /// Cancel an in-progress recording and delete the file.
  static Future<void> cancelRecording() async {
    final path = _currentRecordingPath;
    await stopRecording();

    if (path != null) {
      try {
        final file = File(path);
        if (await file.exists()) {
          await file.delete();
        }
      } catch (e) {
        // Ignore deletion errors
      }
    }
  }
}
