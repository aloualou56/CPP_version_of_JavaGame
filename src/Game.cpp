// Cleaned and consolidated Game.cpp
#include <Game.hpp>
#include <TextureManager.hpp>
#include <Map.hpp>
#include <Camera.hpp>
#include <EnvironmentAssets.hpp>
#include <HUD.hpp>

#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <ECS/ECS.hpp>
#include <ECS/Componets.hpp>
#include <Vector2D.hpp>
#include <Collision.hpp>

// Provide stb_image_write prototype/implementation via a single TU
#include "stb_image_write.h"

#include <vector>


EnvironmentAssets* environmentAssets = nullptr;

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;
Camera* Game::camera = nullptr;
Map* Game::map = nullptr;

Manager manager;
auto& player(manager.addEntity());
auto& wall(manager.addEntity());

// Static pointer to manager for external access
Manager* Game::managerPtr = nullptr;
bool Game::debugMode = false;

Game::Game() {}
Game::~Game() {}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen) {
    Uint32 flags = 0;
    if(fullscreen) flags = SDL_WINDOW_FULLSCREEN;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init succeeded");

        window = SDL_CreateWindow(title, width, height, flags);
        if(window) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window created successfully");
        }

        renderer = SDL_CreateRenderer(window, NULL);
        if(renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer created successfully");
        }

        // Show a simple loading screen so the window appears responsive while textures load
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_FRect box{ (float)(width/2 - 120), (float)(height/2 - 20), 240.0f, 40.0f };
            SDL_RenderFillRect(renderer, &box);
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            SDL_FRect inner{ (float)(width/2 - 110), (float)(height/2 - 10), 220.0f, 20.0f };
            SDL_RenderFillRect(renderer, &inner);
            SDL_RenderPresent(renderer);
            SDL_Delay(50);
            SDL_PumpEvents();
        }

        srand((unsigned int)SDL_GetTicks());
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished initial window/renderer setup");
        isRunning = true;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        isRunning = false;
    }

    map = new Map();
    camera = new Camera(width, height);
    this->hud = new HUD();
    Game::managerPtr = &manager;

    // Count image assets: prefer a packaged manifest, fallback to filesystem scan
    int pngCount = 0;
    SDL_IOStream* io = SDL_IOFromFile("asset_list.txt", "r");
    if (io != nullptr) {
        Sint64 sz = SDL_GetIOSize(io);
        if (sz > 0) {
            std::string buf;
            buf.resize((size_t)sz);
            SDL_ReadIO(io, &buf[0], (size_t)sz);
            SDL_CloseIO(io);
            size_t startpos = 0;
            while (startpos < buf.size()) {
                size_t pos = buf.find('\n', startpos);
                std::string line;
                if (pos == std::string::npos) { line = buf.substr(startpos); startpos = buf.size(); }
                else { line = buf.substr(startpos, pos - startpos); startpos = pos + 1; }
                auto s = line.find_first_not_of(" \t\r\n");
                if (s == std::string::npos) continue;
                auto e = line.find_last_not_of(" \t\r\n");
                std::string path = line.substr(s, e - s + 1);
                std::string ext;
                auto p = path.find_last_of('.');
                if (p != std::string::npos) ext = path.substr(p);
                for (auto &c : ext) c = (char)tolower(c);
                if (ext == ".png" || ext == ".bmp" || ext == ".jpg") pngCount++;
            }
        } else {
            SDL_CloseIO(io);
        }
    } else {
        namespace fs = std::filesystem;
        try {
            for (auto &p : fs::recursive_directory_iterator("sprites")) {
                if (!p.is_regular_file()) continue;
                auto ext = p.path().extension().string();
                for (auto &c : ext) c = (char)tolower(c);
                if (ext == ".png" || ext == ".bmp" || ext == ".jpg") pngCount++;
            }
        } catch (...) {
            pngCount = 0;
        }
    }

    if (pngCount > 0) {
        TextureManager::SetTotalToLoad(pngCount);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Asset manifest/scan found %d image files", pngCount);
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No image assets found by manifest or filesystem scan (pngCount=0)");
    }

    environmentAssets = new EnvironmentAssets(&manager, 50, 50, 96);
    environmentAssets->generateEnvironment();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Environment generation complete");

    // ECS: create player and basic entities
    player.addComponent<PositionComponent>(2400.0f, 2400.0f, 48, 48, 3);
    AnimationComponent& playerAnim = player.addComponent<AnimationComponent>();

    std::vector<std::string> idleAnim = {
        "sprites/characters/cutted-character/standing_sprites/standing_1.png",
        "sprites/characters/cutted-character/standing_sprites/standing_2.png",
        "sprites/characters/cutted-character/standing_sprites/standing_3.png",
        "sprites/characters/cutted-character/standing_sprites/standing_4.png",
        "sprites/characters/cutted-character/standing_sprites/standing_5.png",
        "sprites/characters/cutted-character/standing_sprites/standing_6.png"
    };

    std::vector<std::string> walkAnim = {
        "sprites/characters/cutted-character/walking_sprites/walking_1.png",
        "sprites/characters/cutted-character/walking_sprites/walking_2.png",
        "sprites/characters/cutted-character/walking_sprites/walking_3.png",
        "sprites/characters/cutted-character/walking_sprites/walking_4.png",
        "sprites/characters/cutted-character/walking_sprites/walking_5.png",
        "sprites/characters/cutted-character/walking_sprites/walking_6.png"
    };

    playerAnim.addAnimation("Idle", idleAnim, 200);
    playerAnim.addAnimation("Walk", walkAnim, 100);
    std::vector<std::string> attackAnim = {
        "sprites/characters/cutted-character/fight_sprites/fight_1.png",
        "sprites/characters/cutted-character/fight_sprites/fight_2.png",
        "sprites/characters/cutted-character/fight_sprites/fight_3.png",
        "sprites/characters/cutted-character/fight_sprites/fight_4.png"
    };
    playerAnim.addAnimation("Attack", attackAnim, 80);
    playerAnim.play("Idle");

    player.addComponent<Keyboard>();
    player.addComponent<MouseHandler>();
    player.addComponent<ColliderComponent>("player");
    const int playerMax = 5;
    player.addComponent<HealthComponent>(playerMax);
    this->hud->init(playerMax);
    this->hud->bindHealthComponent(&player.getComponent<HealthComponent>());

    wall.addComponent<PositionComponent>(600.0f, 600.0f, 48, 48, 2);
    wall.addComponent<SpriteComponent>("sprites/tilesets/16x16 set/dirt1.png");
    wall.addComponent<ColliderComponent>("wall");
}

void Game::handleEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_TAB && event.key.repeat == 0) {
                    Game::debugMode = !Game::debugMode;
                    std::cout << "Debug mode: " << (Game::debugMode ? "ON" : "OFF") << std::endl;
                }
                if (event.key.key == SDLK_ESCAPE && event.key.repeat == 0) {
                    isRunning = false;
                }
                if (event.key.key == SDLK_R && event.key.repeat == 0) {
                    if (player.hasComponent<AnimationComponent>()) {
                        player.getComponent<AnimationComponent>().play("Attack", false);
                    }
                }
                break;
            default:
                break;
        }
    }
}

void Game::update() {
    manager.refresh();
    manager.update();

    if (player.hasComponent<PositionComponent>()) {
        Vector2D playerPos = player.getComponent<PositionComponent>().position;
        camera->update(playerPos);
    }

    if (Collision::AABB(player.getComponent<ColliderComponent>().collider, wall.getComponent<ColliderComponent>().collider)) {
        auto& playerPos = player.getComponent<PositionComponent>();
        playerPos.velocity.x *= -1;
        playerPos.velocity.y *= -1;
        unsigned int now = SDL_GetTicks();
        if (now - this->lastDamageTime > 400) {
            this->lastDamageTime = now;
            if (player.hasComponent<HealthComponent>()) {
                auto &hc = player.getComponent<HealthComponent>();
                if (hc.getCurrent() > 0.0f) {
                    hc.takeDamage(1.0f);
                    std::cout << "wall got hit! Player health: " << hc.getCurrent() << std::endl;
                }
            }
        }
    }

    // (rest of update remains as before)
}

void Game::clean() {
    for (const auto &eptr : manager.getEntities()) {
        if (eptr) eptr->destroy();
    }
    manager.refresh();

    if (this->hud) { delete this->hud; this->hud = nullptr; }
    TextureManager::ClearCache();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    delete map;
    delete camera;
    delete environmentAssets;
    SDL_Quit();
    std::cout << "Terminated successfully......." << std::endl;
}

void Game::render() {
    SDL_RenderClear(renderer);
    map->DrawMap(camera);
    manager.draw();
    if (this->hud) this->hud->render();
    SDL_RenderPresent(renderer);
}
