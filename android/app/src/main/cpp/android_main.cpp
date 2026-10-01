/**
 * android_main.cpp - Android entry point for SDL3 game
 * 
 * This file provides the Android-specific main entry point that
 * initializes SDL2 and calls into the game's main loop.
 * 
 * SDL2 for Android uses its own SDLActivity Java class to handle
 * the Android lifecycle, and this native code is loaded as a shared library.
 */

#include <SDL3/SDL.h>
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <string>
#include <Game.hpp>

#define LOG_TAG "SDLGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Frame rate settings
#define FPS 60

// Global game instance
static Game* game = nullptr;

/**
 * Main entry point for the SDL3 Android application.
 * SDL3 requires SDL_main as the entry point function name.
 */
extern "C" int SDL_main(int argc, char* argv[]) {
    LOGI("Starting SDL Game...");
    LOGI("SDL Version: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    
    // Set SDL hints for rendering on Android OpenGL ES (nearest-neighbor
    // filtering for pixel art is set per texture by TextureManager)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    
    // On Android, use the base path which points to the APK's assets
    // This is read-only but contains our game assets
    const char* basePath = SDL_GetBasePath();
    if (basePath) {
        LOGI("Base path (APK assets): %s", basePath);
        chdir(basePath);
    } else {
        LOGE("Failed to get base path: %s", SDL_GetError());
    }
    
    const int FrameDelay = 1000 / FPS;
    Uint64 frameStart;
    int frametime;
    
    // Create game instance
    LOGI("Creating game instance...");
    game = new Game();
    
    // Initialize with Android-appropriate window settings
    // SDL_WINDOWPOS_UNDEFINED lets SDL choose the position
    // Using 0, 0 for window size lets SDL automatically use the device's native resolution
    // This fixes gray screen issues on some Android devices with non-standard resolutions
    LOGI("Calling game->init() with window size 0x0 (native resolution)...");
    game->init("SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
               0, 0, false);
    
    LOGI("Game initialized successfully, entering main loop...");
    
    // Main game loop
    while (game->running()) {
        frameStart = SDL_GetTicks();
        
        game->handleEvents();
        game->update();
        game->render();
        
        // Frame rate limiting
        frametime = (int)(SDL_GetTicks() - frameStart);
        if (FrameDelay > frametime) {
            SDL_Delay(FrameDelay - frametime);
        }
    }
    
    LOGI("Game loop ended, cleaning up...");
    
    // Cleanup
    game->clean();
    delete game;
    game = nullptr;
    
    LOGI("Game terminated successfully.");
    
    return 0;
}
