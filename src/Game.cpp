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

        isRunning = true;
    } else {
        isRunning = false;
    }

    map = new Map();
    camera = new Camera(width, height);
    // Εκθέτει τον manager σε άλλα συστήματα
    Game::managerPtr = &manager;
    
    // Αρχικοποιεί τα περιβαλλοντικά assets (κόσμος 50x50, πλακίδια 96px)
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
        std::cout << "wall got hit!" << std::endl;
    }

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

void Game::render() {

    SDL_RenderClear(renderer);
    // Τι να σχεδιαστεί
    map->DrawMap(camera);
    manager.draw();
    // Τι να σχεδιαστεί
    SDL_RenderPresent(renderer);
    
}