#include <Game.hpp>
#include <TextureManager.hpp>
#include <Map.hpp>
#include <Camera.hpp>
#include <EnvironmentAssets.hpp>
#include <HUD.hpp>

#include <cstdlib>
#include <filesystem>

#include <ECS/ECS.hpp>
#include <ECS/Componets.hpp>
#include <Vector2D.hpp>
#include <Collision.hpp>

// Provide stb_image_write prototype/implementation via a single TU
#include "stb_image_write.h"

#include <vector>
#include <filesystem>


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

    // Temporary test pause: keep window responsive and wait for any key/mouse/quit event
    // so you can interact with the window (debug overlays, walk, spawn particles) before
    // the main loop begins. Remove this block after testing.
    std::cout << "Paused after environment generation. Press any key or click the window to continue..." << std::endl;
    bool _continue_after_pause = false;
    while (!_continue_after_pause) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { _continue_after_pause = true; isRunning = false; break; }
            if (ev.type == SDL_KEYDOWN || ev.type == SDL_MOUSEBUTTONDOWN) { _continue_after_pause = true; break; }
        }
        SDL_Delay(16);
    }

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

    // Automated test scheduling: enable a short scripted test later in update().
    // Automated test is for development only. Disable by default so the player
    // doesn't move without input. Set to `true` here only when actively debugging.
    static bool AUTOMATED_TEST = false;
    static bool testScheduled = false;
    static Uint32 testStartTime = 0;
    if (AUTOMATED_TEST && !testScheduled) {
        testScheduled = true;
        testStartTime = SDL_GetTicks();
        std::cout << "Automated test scheduled (will run for ~6s)" << std::endl;
    }

    // Automated test sequence (temporary). This executes a scripted sequence that:
    // - briefly enables debug overlay, - moves the player across tiles, and - spawns
    //   a few particles behind the player. It's a non-invasive test: it manipulates
    //   the player's PositionComponent directly and creates particle entities.
    static bool testActive = false;
    static Uint32 testStart = 0;
    static Uint32 lastParticleSpawn = 0;
    if (AUTOMATED_TEST && testScheduled) {
        if (!testActive) {
            testActive = true;
            testStart = SDL_GetTicks();
            lastParticleSpawn = testStart;
            std::cout << "Automated test started" << std::endl;
        }
    }

    if (testActive) {
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - testStart;
        // Phase 1: enable debug overlay for first 2000ms
        if (elapsed < 2000) Game::debugMode = true; else Game::debugMode = false;

        // Phase 2: move player to the right across tiles for 4000ms and spawn particles
        if (elapsed >= 1000 && elapsed < 5000) {
            if (player.hasComponent<PositionComponent>()) {
                auto &pos = player.getComponent<PositionComponent>();
                pos.position.x += 1.5f; // move right
            }
            if (now - lastParticleSpawn > 200) {
                lastParticleSpawn = now;
                if (player.hasComponent<PositionComponent>()) {
                    auto &pp = player.getComponent<PositionComponent>();
                    int anchorRow = pp.height;
                    float feetWorldY = pp.position.y + (anchorRow * pp.scale);
                    float spawnX = pp.position.x + (pp.width * pp.scale) / 2.0f - 4.0f;
                    float spawnY = feetWorldY - 8.0f;
                    auto &e = manager.addEntity();
                    e.addComponent<PositionComponent>(spawnX, spawnY, 8, 8, 1);
                    e.addComponent<SpriteComponent>("sprites/particles/dust_particles_small.png");
                    auto &pPos = e.getComponent<PositionComponent>();
                    int behindAnchor = -(pPos.height * pPos.scale);
                    e.getComponent<SpriteComponent>().setAnchorY(behindAnchor);
                    e.addComponent<ParticleComponent>(400.0f, -0.2f + (rand()%100)/500.0f, -0.1f + (rand()%100)/1000.0f, 0.92f);
                }
            }
        }

        // End test after 6000ms
        if (SDL_GetTicks() - testStart > 6000) {
            testActive = false;
            testScheduled = false;
            std::cout << "Automated test finished" << std::endl;
        }
    }

    // Screenshot capture helper: capture current renderer output to PNG using stb
    auto capture_screenshot = [&](const std::string &path)->bool {
        if (!Game::renderer) return false;
        int w=0,h=0;
        SDL_GetRendererOutputSize(Game::renderer, &w, &h);
        if (w <= 0 || h <= 0) return false;
        std::vector<unsigned char> buf((size_t)w * (size_t)h * 4);
        if (SDL_RenderReadPixels(Game::renderer, NULL, SDL_PIXELFORMAT_ABGR8888, buf.data(), w * 4) != 0) {
            return false;
        }
        std::filesystem::create_directories("debug_screenshots");
        int res = stbi_write_png(path.c_str(), w, h, 4, buf.data(), w * 4);
        return (res != 0);
    };

    // Capture three key moments during the automated test (if it ran):
    // - when debug overlay was visible (around 500ms)
    // - mid-move (around 3000ms)
    // - at test end (~6000ms)
    static bool shot1=false, shot2=false, shot3=false;
    if (!testActive && (SDL_GetTicks() - testStart) > 0) {
        // If we recently finished the test, ensure all three screenshots were taken.
        Uint32 sinceEnd = SDL_GetTicks() - (testStart + 6000);
        // Attempt to take any missing shot immediately (some frames still present)
        if (!shot1) { if (capture_screenshot("debug_screenshots/shot_debug.png")) { shot1=true; std::cout<<"Saved debug screenshot"<<std::endl; } }
        if (!shot2) { if (capture_screenshot("debug_screenshots/shot_mid.png"))   { shot2=true; std::cout<<"Saved mid-move screenshot"<<std::endl; } }
        if (!shot3) { if (capture_screenshot("debug_screenshots/shot_end.png"))   { shot3=true; std::cout<<"Saved end screenshot"<<std::endl; } }
    }


}

void Game::clean() {
    // Ensure all managed entities are destroyed while SDL is still valid
    for (const auto &eptr : manager.getEntities()) {
        if (eptr) eptr->destroy();
    }
    manager.refresh();

    if (this->hud) { delete this->hud; this->hud = nullptr; }
    // Clear texture cache while SDL is still active
    TextureManager::ClearCache();
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
