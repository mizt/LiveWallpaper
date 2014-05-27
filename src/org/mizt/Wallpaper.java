// Wallpaper.java based on FlowerService.java https://github.com/harism/android_wallpaper_flowers_ndk/blob/master/src/fi/harism/wallpaper/flowersndk/FlowerService.java
// released under Apache License, Version 2.0 : http://www.apache.org/licenses/LICENSE-2.0

package org.mizt;

import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.service.wallpaper.WallpaperService;
import android.view.Surface;
import android.view.SurfaceHolder;

public final class Wallpaper extends WallpaperService {

	static { System.loadLibrary("wallpaper-jni"); }
    
	public native void wallpaperConnect();
	public native void wallpaperDisconnect();
	public native void wallpaperSetPaused(boolean paused);
	public native void wallpaperSetSurface(Surface surface);
	public native void wallpaperSetSurfaceSize(int width, int height);

	@Override public Engine onCreateEngine() { return new WallpaperEngine(); }

	private final class WallpaperEngine extends Engine implements SharedPreferences.OnSharedPreferenceChangeListener {

		private int _width, _height;

		@Override public void onCreate(SurfaceHolder surfaceHolder) {
			super.onCreate(surfaceHolder);
			wallpaperConnect();
		}

		@Override public void onDestroy() {
			super.onDestroy();
			wallpaperDisconnect();
		}

		@Override public void onOffsetsChanged(float xOffset, float yOffset, float xOffsetStep, float yOffsetStep, int xPixelOffset,int yPixelOffset) {
			super.onOffsetsChanged(xOffset, yOffset, xOffsetStep, yOffsetStep,xPixelOffset, yPixelOffset);
		}

		@Override public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {}

		@Override public void onSurfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			_width  = width;
			_height = height;
			wallpaperSetSurfaceSize(width, height);
		}

		@Override public void onSurfaceCreated(SurfaceHolder holder) {
			wallpaperSetSurface(holder.getSurface());
		}

		@Override public void onSurfaceDestroyed(SurfaceHolder holder) {
			wallpaperSetSurface(null);
		}

		@Override
		public void onVisibilityChanged(boolean visible) {
			super.onVisibilityChanged(visible);
            wallpaperSetPaused(!visible);
			if(visible) {
				wallpaperSetSurface(getSurfaceHolder().getSurface());
				wallpaperSetSurfaceSize(_width,_height);
			}
		}
	}
}
