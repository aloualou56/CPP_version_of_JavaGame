#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Componets.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"


Map* map;

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;

Manager manager;
auto& player(manager.addEntity());
auto& wall(manager.addEntity());

Game::Game() {

}
Game::~Game() {

}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen) {
    int flags = 0;
    if(fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN;
    }

    if(SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "Sub initialised...." << std::endl;
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if(window) {
            std::cout << "Window created successfully" << std::endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if(renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            std::cout << "Renderer created successfully" << std::endl; 
        }

        isRunning = true;
    } else {
        isRunning = false;
    }

    map = new Map();

    //ECS implementation
    

    player.addComponent<PositionComponent>(0, 0);
    player.addComponent<PositionComponent>(3);
    player.addComponent<SpriteComponent>("sprites/characters/cutted-character/standing_sprites/standing_1.png");
    player.addComponent<Keyboard>();
    player.addComponent<ColliderComponent>("player");

    wall.addComponent<PositionComponent>(300.0f, 300.0f, 48, 48, 3);
    wall.addComponent<SpriteComponent>("sprites/tilesets/48wall.png");
    wall.addComponent<ColliderComponent>("wall");


}

void Game::handleEvents() {

    
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
        isRunning = false;
        break;
    
    default:
        break;
    }

}

void Game::update() {

    manager.refresh();
    manager.update();
    if(Collision::AABB(player.getComponent<ColliderComponent>().collider,  wall.getComponent<ColliderComponent>().collider)) {
        player.getComponent<PositionComponent>().velocity * -1;
        std::cout << "wall got hit!" << std::endl;
    }
    std::cout << "(" << player.getComponent<PositionComponent>().position.x << " , " << player.getComponent<PositionComponent>().position.y << ")" << std::endl;
                       
  
}

void Game::render() {

    SDL_RenderClear(renderer);
    //whattorender
    map->DrawMap();
    manager.draw();
    //whattorender
    SDL_RenderPresent(renderer);
    
}

void Game::clean() {
    
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    std::cout << "Terminated successfully......." << std::endl;
}