package org.libsdl.app;

import android.app.Activity;
import android.os.Bundle;

/**
 * Minimal stub of SDLActivity to satisfy compilation of SDLGameActivity.
 * This provides the lifecycle methods that SDLGameActivity overrides.
 *
 * If you later want full SDL functionality, replace this file with the
 * official SDL Java sources from https://github.com/libsdl-org/SDL (android-project/app/src/main/java/org/libsdl/app).
 */
public class SDLActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
    }

    /**
     * Return list of native libraries to load. The real SDLActivity loads
     * SDL libraries based on AndroidManifest metadata; this stub returns an
     * empty list to avoid runtime loading.
     */
    protected String[] getLibraries() {
        return new String[0];
    }
}