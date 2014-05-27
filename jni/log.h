// log.h based on log.h https://github.com/harism/android_wallpaper_flowers_ndk/blob/master/jni/log.h
// released under Apache License, Version 2.0 : http://www.apache.org/licenses/LICENSE-2.0

#pragma once
#include <android/log.h>
#define LOGD(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)

