// thread.h based on gl_thread.h https://github.com/harism/android_wallpaper_flowers_ndk/blob/master/jni/gl_thread.h
// released under Apache License, Version 2.0 : http://www.apache.org/licenses/LICENSE-2.0


#pragma once
#include <EGL/egl.h>
#include <android/native_window.h>

#define GL_CONTEXT_CREATE_ERR { if(egl->context!=EGL_NO_CONTEXT) eglDestroyContext(egl->display, egl->context);\
    if(egl->display!=EGL_NO_DISPLAY) eglTerminate(egl->display);\
    egl->display = EGL_NO_DISPLAY;\
    egl->context = EGL_NO_CONTEXT;\
    return GL_THREAD_FALSE; }

typedef unsigned char gl_thread_bool_t;
#define GL_THREAD_TRUE   1
#define GL_THREAD_FALSE  0


typedef EGLConfig (*gl_ChooseConfig_t)(EGLDisplay display,EGLConfig* configArray, int configCount);
typedef void (*gl_OnRenderFrame_t)(void);
typedef void (*gl_OnSurfaceCreated_t)(void);
typedef void (*gl_OnSurfaceChanged_t)(int32_t width, int32_t height);


typedef struct {
	gl_ChooseConfig_t chooseConfig;
	gl_OnRenderFrame_t onRenderFrame;
	gl_OnSurfaceCreated_t onSurfaceCreated;
	gl_OnSurfaceChanged_t onSurfaceChanged;
} gl_thread_funcs_t;

void gl_ThreadCreate(gl_thread_funcs_t *threadFuncs);
void gl_ThreadDestroy();
void gl_ThreadSetPaused(gl_thread_bool_t paused);
void gl_ThreadSetWindow(ANativeWindow* window);
void gl_ThreadSetWindowSize(int32_t width, int32_t height);