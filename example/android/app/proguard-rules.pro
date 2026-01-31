# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

-keepclasseswithmembernames class * {
    native <methods>;
	*** mNativePtr;
	*** mNativeValue;
	*** mContext;
}

-keep public interface com.serenegiant.camera.ICamera { *; }
-keep public interface com.serenegiant.camera.ICameraControl { *; }
-keep public interface com.serenegiant.camera.ICameraControlEx { *; }
-keep public interface com.serenegiant.camera.ICameraCtrlCompat { *; }
-keep public interface com.serenegiant.camera.IFrameCallback { *; }
-keep public class com.serenegiant.camera.FuncMaxMinDef {
	public *;
}

-keep public class com.serenegiant.camera.CameraState {
	public *;
	protected *;
}

-keep public enum com.serenegiant.camera.RawFrameType {
    public *;
}

-keep class com.serenegiant.camera.Size {
	public *;
}

-keep class com.serenegiant.camera.SizeUtils {
	public *;
}

-keep class com.serenegiant.camera.VideoSizeSelector {
	public *;
}

-keep class * implements android.os.Parcelable {
  public static final android.os.Parcelable$Creator *;
}

-keep public class com.serenegiant.usb.DeviceDetector {
    native <methods>;
	public *;
}

-keep public interface com.serenegiant.usb.DeviceDetector {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.DeviceDetector {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.DeviceDetectorFragment {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.NativeLibLoader {
    public *;
}

-keep public interface com.serenegiant.usb.IUSBCamera { *; }

-keep public class com.serenegiant.usb.uac.UACClient {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.media.Encoder {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.media.FakePipelineVideoEncoder {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.media.PreviewDecoder {
    native <methods>;
	public *;
}

-keep public enum com.serenegiant.usb.uvc.CameraConfig {
    public *;
}

-keep public enum com.serenegiant.usb.uvc.CameraConfig.** {
    public *;
}

-keep public interface com.serenegiant.usb.uvc.IButtonCallback { *; }
-keep public interface com.serenegiant.usb.uvc.IStatusCallback { *; }

-keep public class com.serenegiant.pipeline.IPipeline {*;}
-dontnote com.serenegiant.pipeline.IPipeline

-keep public class com.serenegiant.pipeline.NativeCallback {*;}
-dontnote com.serenegiant.pipeline.NativeCallback

-keep public class com.serenegiant.pipeline.BufferedPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.ConvertPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.DistributePipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.FrameCallbackPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.FrameSavePipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.GLPreviewPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.VkPreviewPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.GLRendererPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.MP4FrameSavePipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.NativeCallbackPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.PipelineSource {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.uvc.PtsCalcPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.SurfaceH264PipelineSource {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.TranscodeH264 {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.pipeline.TranscodeH264PipelineSource {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.uvc.Utils {
	native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.uvc.UVCPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.uvc.UVCUtils {
	public *;
}

-keep public class com.serenegiant.usb.upc.UPCPipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.ptp.PTPController {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.usb.i360.I360Pipeline {
    native <methods>;
	public *;
}

-keep public class com.serenegiant.aandusb.R.**
-keep public class com.serenegiant.aandusb.R.*$*
-keep public class com.serenegiant.aandusb.R.$**
-keep public class com.serenegiant.usb.R.**
-keep public class com.serenegiant.usb.R.*$*
-keep public class com.serenegiant.usb.R.$**

-dontwarn java.lang.invoke.StringConcatFactory

# Please add these rules to your existing keep rules in order to suppress warnings.
# This is generated automatically by the Android Gradle plugin.
-dontwarn com.google.android.play.core.splitcompat.SplitCompatApplication
-dontwarn com.google.android.play.core.splitinstall.SplitInstallException
-dontwarn com.google.android.play.core.splitinstall.SplitInstallManager
-dontwarn com.google.android.play.core.splitinstall.SplitInstallManagerFactory
-dontwarn com.google.android.play.core.splitinstall.SplitInstallRequest$Builder
-dontwarn com.google.android.play.core.splitinstall.SplitInstallRequest
-dontwarn com.google.android.play.core.splitinstall.SplitInstallSessionState
-dontwarn com.google.android.play.core.splitinstall.SplitInstallStateUpdatedListener
-dontwarn com.google.android.play.core.tasks.OnFailureListener
-dontwarn com.google.android.play.core.tasks.OnSuccessListener
-dontwarn com.google.android.play.core.tasks.Task

# DeviceFilter
-keep class org.xmlpull.v1.** { *; }

# Flutter UVC Plugin
-keep public class com.serenegiant.flutter.uvcplugin.UVCManager {
    native <methods>;
	public *;
	protected *;
}
