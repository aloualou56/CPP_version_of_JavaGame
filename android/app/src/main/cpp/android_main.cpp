/**
 * android_main.cpp - Android entry point for SDL2 game
 * 
 * This file provides the Android-specific main entry point that
 * initializes SDL2 and calls into the game's main loop.
 * 
 * SDL2 for Android uses its own SDLActivity Java class to handle
 * the Android lifecycle, and this native code is loaded as a shared library.
 */

#include <SDL.h>
#include <jni.h>
#include <android/log.h>
#include <string>
#include <Game.hpp>

#define LOG_TAG "SDLGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Frame rate settings
#define FPS 30

// Global game instance
static Game* game = nullptr;

/**
 * Main entry point for the SDL2 Android application.
 * SDL2 redefines main() via SDL_main.h, so this function is called
 * automatically when the native library is loaded.
 */
extern "C" int main(int argc, char* argv[]) {
    LOGI("Starting SDL Game...");
    
    const int FrameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frametime;
    
    // Create game instance
    game = new Game();
    
    // Initialize with Android-appropriate window settings
    // SDL_WINDOWPOS_UNDEFINED lets SDL choose the position
    // Window size will be overridden by Android's display
    game->init("SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
               768, 576, false);
    
    LOGI("Game initialized, entering main loop...");
    
    // Main game loop
    while (game->running()) {
        frameStart = SDL_GetTicks();
        
        game->handleEvents();
        game->update();
        game->render();
        
        // Frame rate limiting
        frametime = SDL_GetTicks() - frameStart;
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
