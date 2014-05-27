// thread.cpp based on gl_thread.c https://github.com/harism/android_wallpaper_flowers_ndk/blob/master/jni/gl_thread.c
// released under Apache License, Version 2.0 : http://www.apache.org/licenses/LICENSE-2.0

#include <stdlib.h>
#include <pthread.h>
#include "thread.h"
#include "log.h"

typedef struct {
	EGLDisplay  display;
	EGLContext  context;
	EGLConfig   config;
	EGLSurface  surface;
} gl_thread_egl_t;


gl_thread_bool_t gl_ContextCreate(gl_thread_egl_t *egl, gl_ChooseConfig_t chooseConfig) {

	if(chooseConfig==NULL) return GL_THREAD_FALSE;
    if(egl->display!=EGL_NO_DISPLAY) return GL_THREAD_FALSE;
    if(egl->context!=EGL_NO_CONTEXT) return GL_THREAD_FALSE;
    if(egl->surface!=EGL_NO_SURFACE) return GL_THREAD_FALSE;
    
	egl->display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(egl->display==EGL_NO_DISPLAY) GL_CONTEXT_CREATE_ERR;
    
	if(eglInitialize(egl->display,NULL,NULL)!=EGL_TRUE) GL_CONTEXT_CREATE_ERR;
    
	EGLint numConfigs;
	EGLint configAttrs[] = {

        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        
        EGL_RED_SIZE,    4,
        EGL_GREEN_SIZE,  4,
        EGL_BLUE_SIZE,   4,
        EGL_ALPHA_SIZE,  0,
        EGL_DEPTH_SIZE,  0,
        EGL_STENCIL_SIZE,0,
        EGL_NONE
    };
    
	if(eglChooseConfig(egl->display,configAttrs,NULL,0,&numConfigs)!=EGL_TRUE) GL_CONTEXT_CREATE_ERR;
    
	if(numConfigs<=0) GL_CONTEXT_CREATE_ERR;
    
	EGLint configsSize=numConfigs;
	EGLConfig* configs=(EGLConfig*) malloc(configsSize*sizeof(EGLConfig));
    
	if(eglChooseConfig(egl->display,configAttrs,configs,configsSize,&numConfigs)==EGL_FALSE) {
		free(configs);
        GL_CONTEXT_CREATE_ERR;
	}
    
	egl->config=chooseConfig(egl->display,configs,numConfigs);
	free(configs);
	if(egl->config==NULL) GL_CONTEXT_CREATE_ERR;
    
	EGLint contextAttrs[]={ EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE };
	egl->context=eglCreateContext(egl->display,egl->config,EGL_NO_CONTEXT,contextAttrs);
	if(egl->context==EGL_NO_CONTEXT) GL_CONTEXT_CREATE_ERR;
    
    return GL_THREAD_TRUE;
}

void gl_ContextDestroy(gl_thread_egl_t *egl) {
    if (egl->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(egl->display, EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
		if(egl->context!=EGL_NO_CONTEXT) eglDestroyContext(egl->display,egl->context);
		eglTerminate(egl->display);
	}
	egl->display = EGL_NO_DISPLAY;
	egl->context = EGL_NO_CONTEXT;
	egl->surface = EGL_NO_SURFACE;
}

gl_thread_bool_t gl_SurfaceCreate(gl_thread_egl_t *egl, ANativeWindow *window) {
    if(egl->display==EGL_NO_DISPLAY) return GL_THREAD_FALSE;
    if(egl->context==EGL_NO_CONTEXT) return GL_THREAD_FALSE;
    if(egl->surface!=EGL_NO_SURFACE) return GL_THREAD_FALSE;
    if(egl->config==NULL) return GL_THREAD_FALSE;
    if(window==NULL) return GL_THREAD_FALSE;
	egl->surface=eglCreateWindowSurface(egl->display,egl->config,window,NULL);
    
	if(egl->surface==EGL_NO_SURFACE) return GL_THREAD_FALSE;
    
	if (eglMakeCurrent(egl->display,egl->surface,egl->surface,egl->context) != EGL_TRUE) {
		eglDestroySurface(egl->display, egl->surface);
		egl->surface = EGL_NO_SURFACE;
		return GL_THREAD_FALSE;
	}
    
	return GL_THREAD_TRUE;
}

void gl_SurfaceDestroy(gl_thread_egl_t *egl) {
	if (egl->display != EGL_NO_DISPLAY && egl->surface != EGL_NO_SURFACE) {
		eglMakeCurrent(egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroySurface(egl->display, egl->surface);
	}
	egl->surface = EGL_NO_SURFACE;
}

class GLThread {

    protected:
    
        void process() {
            gl_thread_egl_t egl;
            (&egl)->display = EGL_NO_DISPLAY;
            (&egl)->context = EGL_NO_CONTEXT;
            (&egl)->config  = NULL;
            (&egl)->surface = EGL_NO_SURFACE;
        
            int width = 0, height = 0;
            gl_thread_bool_t hasContext = GL_THREAD_FALSE;
            gl_thread_bool_t hasSurface = GL_THREAD_FALSE;
            gl_thread_bool_t notifySurfaceCreated = GL_THREAD_FALSE;
            gl_thread_bool_t notifySurfaceChanged = GL_THREAD_FALSE;
            
            pthread_mutex_lock(&mutex);
        
            while(!threadExit) {
                while(!threadExit) {
                
                    if(threadPause && hasContext) {
                        gl_SurfaceDestroy(&egl);
                        gl_ContextDestroy(&egl);
                        hasContext = GL_THREAD_FALSE;
                        hasSurface = GL_THREAD_FALSE;
                    }
                    if(windowChanged) {
                        windowChanged = GL_THREAD_FALSE;
                        if(hasSurface) {
                            gl_SurfaceDestroy(&egl);
                            hasSurface = GL_THREAD_FALSE;
                        }
                    }
                    
                    if(!threadPause&&!hasContext) {
                        
                        if((&egl)->surface!=EGL_NO_SURFACE) {
                            eglDestroySurface((&egl)->display,(&egl)->surface);
                            (&egl)->surface = EGL_NO_SURFACE;
                        }
                        
                        hasContext = gl_ContextCreate(&egl,funcs->chooseConfig);
                        if(!hasContext) LOGD("gl_Thread","gl_ContextCreate failed");
                    }
                    
                    if (!threadPause && hasContext && !hasSurface) {
                        hasSurface = gl_SurfaceCreate(&egl,window);
                        notifySurfaceCreated = hasSurface;
                        if(!hasSurface) LOGD("gl_Thread","gl_SurfaceCreate failed");
                    }
                    
                    if(windowSizeChanged) {
                        windowSizeChanged = GL_THREAD_FALSE;
                        width  = windowWidth;
                        height = windowHeight;
                        notifySurfaceChanged = GL_THREAD_TRUE;
                    }
                    
                    if(hasContext&&hasSurface&&width>0&&height>0&&mutexCounter==0) {
                        break;
                    }
                    
                    LOGD("gl_Thread","wait");
                    pthread_cond_wait(&cond,&mutex);

                }
                
                if(threadExit) {
                    break;
                }
                
                if(notifySurfaceCreated) {
                    notifySurfaceCreated = GL_THREAD_FALSE;
                    funcs->onSurfaceCreated();
                }
                
                if(notifySurfaceChanged) {
                    notifySurfaceChanged = GL_THREAD_FALSE;
                    funcs->onSurfaceChanged(width, height);
                }
                
                funcs->onRenderFrame();
                eglSwapBuffers(egl.display, egl.surface);
            }
        
            pthread_mutex_unlock(&mutex);
            
            gl_SurfaceDestroy(&egl);
            gl_ContextDestroy(&egl);
            
            LOGD("gl_Thread","exit");
        }
    
        static void *thread(void *self){
            ((GLThread*)self)->process();
            return 0;
        }
    
	public:
    
        gl_thread_funcs_t *funcs;
    
        pthread_t _thread;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        int mutexCounter;
        
        gl_thread_bool_t threadCreated;
        gl_thread_bool_t threadPause;
        gl_thread_bool_t threadExit;
        gl_thread_bool_t windowChanged;
        gl_thread_bool_t windowSizeChanged;
        
        ANativeWindow *window;
        int32_t windowWidth;
        int32_t windowHeight;
        
        GLThread() {
            
            window = NULL;
            
            threadCreated     = GL_THREAD_TRUE;
            threadPause       = GL_THREAD_FALSE;
            threadExit        = GL_THREAD_FALSE;
            windowChanged     = GL_THREAD_FALSE;
            windowSizeChanged = GL_THREAD_FALSE;
            
            mutexCounter = 0;
            windowWidth = windowHeight = 0;
            
            pthread_cond_init(&cond,NULL);
            pthread_mutex_init(&mutex,NULL);
            
        }
    
        ~GLThread() {
            
            if(threadCreated) {
                threadExit = GL_THREAD_TRUE;
                pthread_cond_signal(&cond);
                pthread_join(_thread, NULL);
                
                pthread_mutex_lock(&mutex);
                while (mutexCounter>0) pthread_cond_wait(&cond,&mutex);
                pthread_mutex_unlock(&mutex);
                pthread_cond_destroy(&cond);
                pthread_mutex_destroy(&mutex);
                if(window) ANativeWindow_release(window);
            }
        }
    
    
        void start(gl_thread_funcs_t *threadFuncs) {
            funcs = threadFuncs;
            pthread_create(&_thread,NULL,thread,(void *)this);
        }
    
        gl_thread_bool_t isRunning() {
            if (threadCreated&&!threadExit) {
                return GL_THREAD_TRUE;
            }
            return GL_THREAD_FALSE;
        }
    
    
        void lock() {
            if(isRunning()) {
                ++mutexCounter;
                pthread_mutex_lock(&mutex);
            }
        }
        
        
        void unlock() {
            if(isRunning()) {
                --mutexCounter;
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&cond);
            }
        }
    
        void paused(gl_thread_bool_t p) {
            if(isRunning()) {
                lock();
                    threadPause = p;
                unlock();
            }
        }
    
    
        void setWindow(ANativeWindow *w) {
        
            if(!isRunning()) {
                ANativeWindow_release(w);
                return;
            }
            
            lock();
                if(window != w) {
                    if(window) ANativeWindow_release(window);
                    window = w;
                    windowChanged = GL_THREAD_TRUE;
                    windowWidth = windowHeight = 0;
                    windowSizeChanged = GL_THREAD_TRUE;
                }
                else if(w) {
                    ANativeWindow_release(w);
                }
            unlock();
        }
        
    
        void setWindowSize(int32_t width, int32_t height) {
        
            if(!isRunning()) { return; }
            
            lock();
                if(windowWidth!=width||windowHeight!=height) {
                    windowWidth  = width;
                    windowHeight = height;
                    windowSizeChanged = GL_THREAD_TRUE;
                }
            unlock();
        }
};

GLThread *glThread = NULL;

void gl_ThreadCreate(gl_thread_funcs_t *threadFuncs) {
    gl_ThreadDestroy();
    glThread = new GLThread();
    glThread->start(threadFuncs);
}

void gl_ThreadDestroy() {
    if(glThread) {
        delete glThread;
        glThread = NULL;
    }
}

void gl_ThreadSetPaused(gl_thread_bool_t paused) {
	glThread->paused(paused);
}

void gl_ThreadSetWindow(ANativeWindow *window) {
    glThread->setWindow(window);
}

void gl_ThreadSetWindowSize(int32_t width, int32_t height) {
	glThread->setWindowSize(width,height);
}