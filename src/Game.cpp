#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "Camera.hpp"
#include "EnvironmentAssets.hpp"
#include "HUD.hpp"

#include <cstdlib>
#include <filesystem>

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

// Ορίζει στατική μεταβλητή δείκτη στον manager για εξωτερική πρόσβαση
Manager* Game::managerPtr = nullptr;
// Διακόπτης debug αρχικά απενεργοποιημένος
bool Game::debugMode = false;

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

        // Show a simple loading screen so the window appears responsive while textures load
        if (renderer) {
            // black background
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
            SDL_RenderClear(renderer);
            // simple white loading bar box in center
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect box{ width/2 - 120, height/2 - 20, 240, 40 };
            SDL_RenderFillRect(renderer, &box);
            // a smaller dark bar inside to look like a progress area
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            SDL_Rect inner{ width/2 - 110, height/2 - 10, 220, 20 };
            SDL_RenderFillRect(renderer, &inner);
            SDL_RenderPresent(renderer);
            SDL_Delay(50); // give OS a moment to present the window
            SDL_PumpEvents();
        }

        // seed RNG for particle randomness
        srand((unsigned int)SDL_GetTicks());

        isRunning = true;
    } else {
        isRunning = false;
    }

    map = new Map();
    camera = new Camera(width, height);
    // Create HUD
    this->hud = new HUD();
    // HUD will be initialised after player creation so it can be bound to the HealthComponent
    // Εκθέτει τον manager σε άλλα συστήματα
    Game::managerPtr = &manager;
    
    // Αρχικοποιεί τα περιβαλλοντικά assets (κόσμος 50x50, πλακίδια 96px)
    // Before creating many textures, scan sprite folder to estimate work for loading screen
    namespace fs = std::filesystem;
    int pngCount = 0;
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
    if (pngCount > 0) TextureManager::SetTotalToLoad(pngCount);

    environmentAssets = new EnvironmentAssets(&manager, 50, 50, 96);
    environmentAssets->generateEnvironment();

    // Υλοποίηση ECS - Δημιουργία παίκτη με animations
    player.addComponent<PositionComponent>(2400.0f, 2400.0f, 48, 48, 3);  // Ξεκινά στο κέντρο του κόσμου, κλίμακα 3x (μεγαλύτερος)

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
    // Animation επίθεσης / μάχης (μία εκτέλεση)
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
    // Add health via ECS and bind HUD to it
    const int playerMax = 5; // default player max health
    player.addComponent<HealthComponent>(playerMax);
    // Init HUD now that health component exists
    this->hud->init(playerMax);
    this->hud->bindHealthComponent(&player.getComponent<HealthComponent>());

    wall.addComponent<PositionComponent>(600.0f, 600.0f, 48, 48, 2);
    wall.addComponent<SpriteComponent>("sprites/tilesets/16x16 set/dirt1.png");
    wall.addComponent<ColliderComponent>("wall");

}

void Game::handleEvents() {
    // Επεξεργασία όλων των εκκρεμών SDL γεγονότων; χειρισμός εξόδου και keydown για επίθεση
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                // Εναλλαγή overlay/logging debug με το πλήκτρο Tab
                if (event.key.keysym.sym == SDLK_TAB && event.key.repeat == 0) {
                    Game::debugMode = !Game::debugMode;
                    std::cout << "Debug mode: " << (Game::debugMode ? "ON" : "OFF") << std::endl;
                }
                // Close game with Escape key
                if (event.key.keysym.sym == SDLK_ESCAPE && event.key.repeat == 0) {
                    isRunning = false;
                }
                // Εκκίνηση μιας φοράς επίθεσης όταν πατηθεί R (παραβλέπει επαναλήψεις)
                if (event.key.keysym.sym == SDLK_r && event.key.repeat == 0) {
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
    
    // Ενημέρωση της κάμερας ώστε να ακολουθεί τον παίκτη
    if (player.hasComponent<PositionComponent>()) {
        Vector2D playerPos = player.getComponent<PositionComponent>().position;
        camera->update(playerPos);
    }
    
    if(Collision::AABB(player.getComponent<ColliderComponent>().collider,  wall.getComponent<ColliderComponent>().collider)) {
        auto& playerPos = player.getComponent<PositionComponent>();
        playerPos.velocity.x *= -1;
        playerPos.velocity.y *= -1;
        // Apply damage with a small cooldown to avoid draining health instantly
        unsigned int now = SDL_GetTicks();
        if (now - this->lastDamageTime > 400) {
            this->lastDamageTime = now;
            if (player.hasComponent<HealthComponent>()) {
                auto &hc = player.getComponent<HealthComponent>();
                if (hc.getCurrent() > 0.0f) {
                    hc.takeDamage(1.0f); // subtract one full heart
                    std::cout << "wall got hit! Player health: " << hc.getCurrent() << std::endl;
                }
            }
        }
    }

}

void Game::clean() {
    // Ensure all managed entities are destroyed while SDL is still valid
    for (const auto &eptr : manager.getEntities()) {
        if (eptr) eptr->destroy();
    }
    manager.refresh();

    if (this->hud) { delete this->hud; this->hud = nullptr; }
    // Destroy renderer and window after components/textures have been freed
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
    // Τι να σχεδιαστεί
    map->DrawMap(camera);
    manager.draw();
    // Draw HUD (hearts in corner)
    if (this->hud) this->hud->render();
    // Τι να σχεδιαστεί
    SDL_RenderPresent(renderer);
    
}
