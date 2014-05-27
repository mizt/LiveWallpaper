LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libwallpaper-jni
LOCAL_CFLAGS    += -Wall -Werror -Wextra -fexceptions

LOCAL_SRC_FILES := main.cpp \
                   thread.cpp

LOCAL_LDLIBS    := -landroid \
                   -llog \
                   -lEGL \
                   -lGLESv2 \
                   -lstdc++

include $(BUILD_SHARED_LIBRARY)
