#include <SDL3/SDL.h>
#include <Game.hpp>

#define FPS 60

Game *game = nullptr;

int main(int argc, char* argv[]) {
    (void)argc; (void)argv; // SDL3 doesn't require SDL_main for all platforms

    const int FrameDelay = 1000 / FPS;

    Uint64 frameStart;
    Uint64 frametime;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting game process (main)");
    game = new Game();

    game->init("prototype", 0, 0, 768, 576, false);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game::init returned (main)");

    while(game->running()) {

        frameStart = SDL_GetTicks();

        game->handleEvents();
        game->update();
        game->render();

        frametime = SDL_GetTicks() - frameStart;
        if((Uint64)FrameDelay > frametime) {
            SDL_Delay((Uint32)(FrameDelay - frametime));
        }
    } 

    game->clean();
 
    return 0;
}
