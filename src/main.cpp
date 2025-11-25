#include <SDL2/SDL.h>
#include <Game.hpp>

#define FPS 30

Game *game = nullptr;

int main(int argc, char* argv[]) {

    const int FrameDelay = 1000 / FPS;

    Uint32 frameStart;
    int frametime;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting game process (main)");
    game = new Game();

    game->init("prototype", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 768, 576, false);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game::init returned (main)");

    while(game->running()) {

        frameStart = SDL_GetTicks();

        game->handleEvents();
        game->update();
        game->render();

        frametime = SDL_GetTicks() - frameStart;
        if(FrameDelay > frametime) {
            SDL_Delay(FrameDelay - frametime);
        }
    } 

    game->clean();
 
    return 0;
}
