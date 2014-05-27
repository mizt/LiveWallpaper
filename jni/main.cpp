// main.cpp based on flowers_main.c https://github.com/harism/android_wallpaper_flowers_ndk/blob/master/jni/flowers_main.c
// released under Apache License, Version 2.0 : http://www.apache.org/licenses/LICENSE-2.0

#include <stdlib.h>
#include <jni.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "thread.h"

#define WALLPAPER_EXTERN(func) Java_org_mizt_Wallpaper_ ## func
#define UNUSED __attribute__ ((unused))

int _hostCount;

#define THREAD_FUNCS wallpaper_thread_funcs
gl_thread_funcs_t THREAD_FUNCS;

void onRenderFrame() {
    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void onSurfaceChanged(int32_t width, int32_t height) {
    glViewport(0,0,width,height);
}

void onSurfaceCreated() {

}

EGLConfig chooseConfig(EGLDisplay display, EGLConfig* configArray, int configCount) {
    
	EGLConfig retConfig = NULL;
	
    int highestSum = 0;
	int highestSub = 0;
    
    for(int idx=0; idx<configCount; ++idx) {
        
		EGLConfig config = configArray[idx];
		
        int r=0, g=0, b = 0, a = 0;
        int d=0, s=0;
        
		eglGetConfigAttrib(display,config,EGL_RED_SIZE,&r);
		eglGetConfigAttrib(display,config,EGL_GREEN_SIZE,&g);
		eglGetConfigAttrib(display,config,EGL_BLUE_SIZE,&b);
		eglGetConfigAttrib(display,config,EGL_ALPHA_SIZE,&a);
		eglGetConfigAttrib(display,config,EGL_DEPTH_SIZE,&d);
		eglGetConfigAttrib(display,config,EGL_STENCIL_SIZE,&s);
        
		int sum = r+g+b;
		int sub = a+d+s;
        
		if(sum>highestSum||(sum==highestSum&&sub<highestSub)) {
			retConfig = config;
			highestSum = sum;
			highestSub = sub;
		}
	}
	return retConfig;
}

extern "C" {

void WALLPAPER_EXTERN(wallpaperConnect(UNUSED JNIEnv *env)) {
	if(_hostCount==0) {
		THREAD_FUNCS.chooseConfig     = chooseConfig;
		THREAD_FUNCS.onRenderFrame    = onRenderFrame;
		THREAD_FUNCS.onSurfaceChanged = onSurfaceChanged;
		THREAD_FUNCS.onSurfaceCreated = onSurfaceCreated;
		gl_ThreadCreate(&THREAD_FUNCS);
	}
	++_hostCount;
}

void WALLPAPER_EXTERN(wallpaperDisconnect(UNUSED JNIEnv *env)) {
    if(_hostCount>=1) {
        if(_hostCount==1) gl_ThreadDestroy();
        --_hostCount;
    }
}

void WALLPAPER_EXTERN(wallpaperSetPaused(UNUSED JNIEnv *env, UNUSED jobject obj, jboolean paused)) {
	gl_ThreadSetPaused((paused==JNI_TRUE)?GL_THREAD_TRUE:GL_THREAD_FALSE);
}

void WALLPAPER_EXTERN(wallpaperSetSurface(JNIEnv *env, UNUSED jobject obj, jobject surface)) {
	gl_ThreadSetWindow((surface)?ANativeWindow_fromSurface(env,surface):NULL);
}

void WALLPAPER_EXTERN(wallpaperSetSurfaceSize(UNUSED JNIEnv *env, UNUSED jobject obj, jint width, jint height)) {
	gl_ThreadSetWindowSize(width, height);
}

}
