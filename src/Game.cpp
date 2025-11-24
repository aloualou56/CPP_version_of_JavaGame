#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "Camera.hpp"
#include "EnvironmentAssets.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Componets.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"


EnvironmentAssets* environmentAssets;

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;
Camera* Game::camera = nullptr;
Map* Game::map = nullptr;

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
    camera = new Camera(width, height);
    
    // Initialize environment assets (50x50 world, 96px tiles)
    environmentAssets = new EnvironmentAssets(&manager, 50, 50, 96);
    environmentAssets->generateEnvironment();

    //ECS implementation - Setup player with animations
    player.addComponent<PositionComponent>(2400.0f, 2400.0f, 48, 48, 2);  // Start in center of world, scale 2x
    player.addComponent<SpriteComponent>("sprites/characters/cutted-character/standing_sprites/standing_1.png");
    player.addComponent<Keyboard>();
    player.addComponent<MouseHandler>();
    player.addComponent<ColliderComponent>("player");

    wall.addComponent<PositionComponent>(600.0f, 600.0f, 48, 48, 2);
    wall.addComponent<SpriteComponent>("sprites/tilesets/16x16 set/dirt1.png");
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
    
    // Update camera to follow player
    if (player.hasComponent<PositionComponent>()) {
        Vector2D playerPos = player.getComponent<PositionComponent>().position;
        camera->update(playerPos);
    }
    
    if(Collision::AABB(player.getComponent<ColliderComponent>().collider,  wall.getComponent<ColliderComponent>().collider)) {
        auto& playerPos = player.getComponent<PositionComponent>();
        playerPos.velocity.x *= -1;
        playerPos.velocity.y *= -1;
        std::cout << "wall got hit!" << std::endl;
    }
                       
  
}

void Game::render() {

    SDL_RenderClear(renderer);
    //whattorender
    map->DrawMap(camera);
    manager.draw();
    //whattorender
    SDL_RenderPresent(renderer);
    
}

void Game::clean() {
    
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    delete map;
    delete camera;
    delete environmentAssets;
    SDL_Quit();
    std::cout << "Terminated successfully......." << std::endl;
}