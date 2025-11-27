package com.aloualou.cppgame;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowInsetsController;

import org.libsdl.app.SDLActivity;

/**
 * Main Activity for the SDL Game.
 * Extends SDLActivity which handles SDL initialization and lifecycle.
 * 
 * The native library "game" will be loaded automatically by SDLActivity
 * based on the android.app.lib_name metadata in AndroidManifest.xml.
 */
public class SDLGameActivity extends SDLActivity {
    private static final String TAG = "SDLGameActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate() called");
        super.onCreate(savedInstanceState);
        
        // Keep screen on during gameplay
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        
        // Hide system UI for immersive mode
        hideSystemUI();
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemUI();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    /**
     * Hide system UI for a full immersive experience.
     * Uses WindowInsetsController for API 30+ with fallback to deprecated methods.
     */
    @SuppressWarnings("deprecation")
    private void hideSystemUI() {
        Window window = getWindow();
        View decorView = window.getDecorView();
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // API 30+ uses WindowInsetsController
            WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.hide(android.view.WindowInsets.Type.statusBars() 
                    | android.view.WindowInsets.Type.navigationBars());
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            // Fallback for older APIs
            decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
            );
        }
    }

    /**
     * Return the name of the native library to load
     * This matches the library name in CMakeLists.txt
     */
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "SDL3_image",
            "game"
        };
    }
}
