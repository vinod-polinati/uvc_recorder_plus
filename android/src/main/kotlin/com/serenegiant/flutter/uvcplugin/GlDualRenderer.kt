/**
 * GL Renderer for dual surface output (preview + recording)
 * 
 * Renders the UVC camera SurfaceTexture to both:
 * 1. Preview surface (Flutter texture)
 * 2. Recording surface (MediaCodec input)
 */
package com.serenegiant.flutter.uvcplugin

import android.graphics.SurfaceTexture
import android.opengl.EGL14
import android.opengl.EGLConfig
import android.opengl.EGLContext
import android.opengl.EGLDisplay
import android.opengl.EGLSurface
import android.opengl.GLES11Ext
import android.opengl.GLES20
import android.opengl.Matrix
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.view.Surface
import androidx.annotation.Keep
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer

/**
 * Manages OpenGL ES rendering to multiple surfaces
 */
@Keep
class GlDualRenderer(
    private val sourceTexture: SurfaceTexture,
    private val previewSurface: Surface,
    private val videoWidth: Int,
    private val videoHeight: Int
) : SurfaceTexture.OnFrameAvailableListener {
    
    companion object {
        private const val TAG = "GlDualRenderer"
        private const val DEBUG = true
        
        // Vertex shader for rendering SurfaceTexture
        private const val VERTEX_SHADER = """
            uniform mat4 uMVPMatrix;
            uniform mat4 uSTMatrix;
            attribute vec4 aPosition;
            attribute vec4 aTextureCoord;
            varying vec2 vTextureCoord;
            void main() {
                gl_Position = uMVPMatrix * aPosition;
                vTextureCoord = (uSTMatrix * aTextureCoord).xy;
            }
        """
        
        // Fragment shader for external OES texture
        private const val FRAGMENT_SHADER = """
            #extension GL_OES_EGL_image_external : require
            precision mediump float;
            varying vec2 vTextureCoord;
            uniform samplerExternalOES sTexture;
            void main() {
                gl_FragColor = texture2D(sTexture, vTextureCoord);
            }
        """
        
        // Vertex data for full-screen quad
        private val VERTICES = floatArrayOf(
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f
        )
    }
    
    // EGL state
    private var eglDisplay: EGLDisplay = EGL14.EGL_NO_DISPLAY
    private var eglContext: EGLContext = EGL14.EGL_NO_CONTEXT
    private var eglConfig: EGLConfig? = null
    private var previewEglSurface: EGLSurface = EGL14.EGL_NO_SURFACE
    private var recordEglSurface: EGLSurface = EGL14.EGL_NO_SURFACE
    
    // Recording surface (optional)
    private var recordingSurface: Surface? = null
    
    // OpenGL program and handles
    private var program = 0
    private var textureId = 0
    private var aPositionHandle = 0
    private var aTextureCoordHandle = 0
    private var uMVPMatrixHandle = 0
    private var uSTMatrixHandle = 0
    
    // Matrices
    private val mvpMatrix = FloatArray(16)
    private val stMatrix = FloatArray(16)
    
    // Buffers
    private var vertexBuffer: FloatBuffer? = null
    
    // Rendering thread
    private var renderThread: HandlerThread? = null
    private var renderHandler: Handler? = null
    private var isRunning = false
    
    init {
        Matrix.setIdentityM(mvpMatrix, 0)
        Matrix.setIdentityM(stMatrix, 0)
        
        // Create vertex buffer
        val bb = ByteBuffer.allocateDirect(VERTICES.size * 4)
        bb.order(ByteOrder.nativeOrder())
        vertexBuffer = bb.asFloatBuffer()
        vertexBuffer?.put(VERTICES)
        vertexBuffer?.position(0)
    }
    
    /**
     * Start rendering
     */
    fun start() {
        if (isRunning) return
        
        renderThread = HandlerThread("GlDualRenderer").apply { start() }
        renderHandler = Handler(renderThread!!.looper)
        
        renderHandler?.post {
            try {
                initEgl()
                initGl()
                sourceTexture.setOnFrameAvailableListener(this, renderHandler)
                isRunning = true
                if (DEBUG) Log.d(TAG, "Renderer started")
            } catch (e: Exception) {
                Log.e(TAG, "Failed to start renderer", e)
            }
        }
    }
    
    /**
     * Stop rendering
     */
    fun stop() {
        isRunning = false
        sourceTexture.setOnFrameAvailableListener(null)
        
        renderHandler?.post {
            releaseGl()
            releaseEgl()
        }
        
        renderThread?.quitSafely()
        renderThread = null
        renderHandler = null
    }
    
    /**
     * Set recording surface
     */
    fun setRecordingSurface(surface: Surface?) {
        renderHandler?.post {
            // Release old surface
            if (recordEglSurface != EGL14.EGL_NO_SURFACE) {
                EGL14.eglDestroySurface(eglDisplay, recordEglSurface)
                recordEglSurface = EGL14.EGL_NO_SURFACE
            }
            
            recordingSurface = surface
            
            // Create new surface
            if (surface != null && surface.isValid) {
                recordEglSurface = EGL14.eglCreateWindowSurface(
                    eglDisplay, eglConfig, surface, intArrayOf(EGL14.EGL_NONE), 0
                )
                if (DEBUG) Log.d(TAG, "Recording surface created")
            }
        }
    }
    
    override fun onFrameAvailable(surfaceTexture: SurfaceTexture?) {
        if (!isRunning) return
        
        renderHandler?.post {
            try {
                // Update texture
                sourceTexture.updateTexImage()
                sourceTexture.getTransformMatrix(stMatrix)
                
                // Render to preview surface
                renderToSurface(previewEglSurface, videoWidth, videoHeight)
                
                // Render to recording surface if available
                if (recordEglSurface != EGL14.EGL_NO_SURFACE) {
                    renderToSurface(recordEglSurface, videoWidth, videoHeight)
                }
            } catch (e: Exception) {
                if (DEBUG) Log.w(TAG, "Render error", e)
            }
        }
    }
    
    private fun renderToSurface(eglSurface: EGLSurface, width: Int, height: Int) {
        if (!EGL14.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
            return
        }
        
        GLES20.glViewport(0, 0, width, height)
        GLES20.glClearColor(0f, 0f, 0f, 1f)
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
        
        GLES20.glUseProgram(program)
        
        // Set vertex attributes
        vertexBuffer?.position(0)
        GLES20.glVertexAttribPointer(aPositionHandle, 3, GLES20.GL_FLOAT, false, 20, vertexBuffer)
        GLES20.glEnableVertexAttribArray(aPositionHandle)
        
        vertexBuffer?.position(3)
        GLES20.glVertexAttribPointer(aTextureCoordHandle, 2, GLES20.GL_FLOAT, false, 20, vertexBuffer)
        GLES20.glEnableVertexAttribArray(aTextureCoordHandle)
        
        // Set uniforms
        GLES20.glUniformMatrix4fv(uMVPMatrixHandle, 1, false, mvpMatrix, 0)
        GLES20.glUniformMatrix4fv(uSTMatrixHandle, 1, false, stMatrix, 0)
        
        // Bind texture
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0)
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId)
        
        // Draw
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
        
        // Swap buffers
        EGL14.eglSwapBuffers(eglDisplay, eglSurface)
    }
    
    private fun initEgl() {
        eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY)
        if (eglDisplay == EGL14.EGL_NO_DISPLAY) {
            throw RuntimeException("No EGL display")
        }
        
        val version = IntArray(2)
        if (!EGL14.eglInitialize(eglDisplay, version, 0, version, 1)) {
            throw RuntimeException("Failed to initialize EGL")
        }
        
        val configAttribs = intArrayOf(
            EGL14.EGL_RED_SIZE, 8,
            EGL14.EGL_GREEN_SIZE, 8,
            EGL14.EGL_BLUE_SIZE, 8,
            EGL14.EGL_ALPHA_SIZE, 8,
            EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
            EGL14.EGL_SURFACE_TYPE, EGL14.EGL_WINDOW_BIT,
            EGL14.EGL_NONE
        )
        
        val configs = arrayOfNulls<EGLConfig>(1)
        val numConfigs = IntArray(1)
        if (!EGL14.eglChooseConfig(eglDisplay, configAttribs, 0, configs, 0, 1, numConfigs, 0)) {
            throw RuntimeException("Failed to choose EGL config")
        }
        eglConfig = configs[0]
        
        val contextAttribs = intArrayOf(
            EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL14.EGL_NONE
        )
        
        eglContext = EGL14.eglCreateContext(eglDisplay, eglConfig, EGL14.EGL_NO_CONTEXT, contextAttribs, 0)
        if (eglContext == EGL14.EGL_NO_CONTEXT) {
            throw RuntimeException("Failed to create EGL context")
        }
        
        previewEglSurface = EGL14.eglCreateWindowSurface(
            eglDisplay, eglConfig, previewSurface, intArrayOf(EGL14.EGL_NONE), 0
        )
    }
    
    private fun initGl() {
        EGL14.eglMakeCurrent(eglDisplay, previewEglSurface, previewEglSurface, eglContext)
        
        // Create program
        val vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, VERTEX_SHADER)
        val fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, FRAGMENT_SHADER)
        
        program = GLES20.glCreateProgram()
        GLES20.glAttachShader(program, vertexShader)
        GLES20.glAttachShader(program, fragmentShader)
        GLES20.glLinkProgram(program)
        
        aPositionHandle = GLES20.glGetAttribLocation(program, "aPosition")
        aTextureCoordHandle = GLES20.glGetAttribLocation(program, "aTextureCoord")
        uMVPMatrixHandle = GLES20.glGetUniformLocation(program, "uMVPMatrix")
        uSTMatrixHandle = GLES20.glGetUniformLocation(program, "uSTMatrix")
        
        // Create external texture
        val textures = IntArray(1)
        GLES20.glGenTextures(1, textures, 0)
        textureId = textures[0]
        
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId)
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR)
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR)
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE)
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE)
    }
    
    private fun loadShader(type: Int, source: String): Int {
        val shader = GLES20.glCreateShader(type)
        GLES20.glShaderSource(shader, source)
        GLES20.glCompileShader(shader)
        return shader
    }
    
    private fun releaseGl() {
        if (program != 0) {
            GLES20.glDeleteProgram(program)
            program = 0
        }
        if (textureId != 0) {
            GLES20.glDeleteTextures(1, intArrayOf(textureId), 0)
            textureId = 0
        }
    }
    
    private fun releaseEgl() {
        if (eglDisplay != EGL14.EGL_NO_DISPLAY) {
            EGL14.eglMakeCurrent(eglDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT)
            
            if (previewEglSurface != EGL14.EGL_NO_SURFACE) {
                EGL14.eglDestroySurface(eglDisplay, previewEglSurface)
                previewEglSurface = EGL14.EGL_NO_SURFACE
            }
            if (recordEglSurface != EGL14.EGL_NO_SURFACE) {
                EGL14.eglDestroySurface(eglDisplay, recordEglSurface)
                recordEglSurface = EGL14.EGL_NO_SURFACE
            }
            if (eglContext != EGL14.EGL_NO_CONTEXT) {
                EGL14.eglDestroyContext(eglDisplay, eglContext)
                eglContext = EGL14.EGL_NO_CONTEXT
            }
            EGL14.eglTerminate(eglDisplay)
            eglDisplay = EGL14.EGL_NO_DISPLAY
        }
    }
}
