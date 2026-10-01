// Cleaned and consolidated Game.cpp
#include <Game.hpp>
#include <TextureManager.hpp>
#include <Map.hpp>
#include <Camera.hpp>
#include <EnvironmentAssets.hpp>
#include <HUD.hpp>
#include <SevenSegment.hpp>

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
#include <string>

#include <cmath>


EnvironmentAssets* environmentAssets = nullptr;

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;
Camera* Game::camera = nullptr;
Map* Game::map = nullptr;

// Screen and touch/multitouch state (Android)
int Game::screenWidth = 0;
int Game::screenHeight = 0;
long long Game::movementFingerId = -1;
long long Game::attackFingerId = -1;
bool Game::movementActive = false;
float Game::movementDX = 0.0f;
float Game::movementDY = 0.0f;
bool Game::attackActive = false;
SDL_Haptic *Game::haptic = nullptr;

Manager manager;
auto& player(manager.addEntity());

// Static pointer to manager for external access
Manager* Game::managerPtr = nullptr;
Entity* Game::playerEntity = nullptr;
bool Game::debugMode = false;
bool Game::gameStarted = false;

Game::Game() {}
Game::~Game() {}

// (Re)loads the four player animation sets from a character art folder -
// either "cutted-character" (boy, the default) or "cutted-character-girl" -
// both keep the same file layout/frame count, just different art, so the
// same loading logic works for either. Calling this again on an
// already-initialized AnimationComponent (from the character-select screen)
// simply overwrites the existing "Idle"/"Walk"/"Attack"/"Dead" entries with
// textures from the new folder.
static void loadPlayerAnimations(AnimationComponent& anim, const std::string& folder) {
    std::string base = "sprites/characters/" + folder + "/";

    std::vector<std::string> idleAnim = {
        base + "standing_sprites/standing_1.png", base + "standing_sprites/standing_2.png",
        base + "standing_sprites/standing_3.png", base + "standing_sprites/standing_4.png",
        base + "standing_sprites/standing_5.png", base + "standing_sprites/standing_6.png"
    };
    std::vector<std::string> walkAnim = {
        base + "walking_sprites/walking_1.png", base + "walking_sprites/walking_2.png",
        base + "walking_sprites/walking_3.png", base + "walking_sprites/walking_4.png",
        base + "walking_sprites/walking_5.png", base + "walking_sprites/walking_6.png"
    };
    std::vector<std::string> attackAnim = {
        base + "fight_sprites/fight_1.png", base + "fight_sprites/fight_2.png",
        base + "fight_sprites/fight_3.png", base + "fight_sprites/fight_4.png"
    };
    std::vector<std::string> deadAnim = {
        base + "dead_sprites/dead_1.png", base + "dead_sprites/dead_2.png", base + "dead_sprites/dead_3.png"
    };

    anim.addAnimation("Idle", idleAnim, 200);
    anim.addAnimation("Walk", walkAnim, 100);
    anim.addAnimation("Attack", attackAnim, 80);
    anim.addAnimation("Dead", deadAnim, 300);
}

// --- World entity factories -------------------------------------------------
// A hostile slime: wanders until the player enters detectionRange, then
// chases and deals contact damage. slime.png is a spritesheet, not a
// ready-to-draw image, so the two idle-squish frames are cropped out of it
// once via TextureManager::LoadTextureRegion and cached (mirrors Java's
// Enemy.loadImage(), which does the same crop at runtime).
static Entity& spawnEnemy(Manager& mgr, float x, float y) {
    auto& e = mgr.addEntity();
    e.addComponent<PositionComponent>(x, y, 32, 32, 2); // 64x64 world box, matches Java's enemy solidArea
    e.getComponent<PositionComponent>().speed = 2;

    AnimationComponent& anim = e.addComponent<AnimationComponent>();
    TextureManager::LoadTextureRegion("sprites/characters/slime.png", "sprites/characters/slime_frame_a", 0, 0, 32, 32);
    TextureManager::LoadTextureRegion("sprites/characters/slime.png", "sprites/characters/slime_frame_b", 32, 0, 32, 32);
    std::vector<std::string> idle = { "sprites/characters/slime_frame_a", "sprites/characters/slime_frame_b" };
    anim.addAnimation("Idle", idle, 300);
    anim.play("Idle");

    // Rescaled onto a small internal HP pool (see EnemyAIComponent/AttackComponent
    // comments) so the hits-to-kill ratio still matches Java's 40hp/25dmg (~2 hits).
    e.addComponent<CombatComponent>(2.0f, "enemy");
    e.addComponent<EnemyAIComponent>();
    return e;
}

// A friendly, non-hostile wanderer. Reuses the player's own animation art,
// tinted blue, since there's no dedicated NPC sprite sheet - same approach
// Java's NPC.java uses.
static Entity& spawnNPC(Manager& mgr, float x, float y) {
    auto& e = mgr.addEntity();
    e.addComponent<PositionComponent>(x, y, 48, 48, 3); // same on-screen size as the player
    e.getComponent<PositionComponent>().speed = 1;

    AnimationComponent& anim = e.addComponent<AnimationComponent>();
    TextureManager::LoadTintedTexture("sprites/characters/cutted-character/standing_sprites/standing_1.png", "sprites/characters/npc_idle_tinted", 60, 110, 220, 0.45f);
    TextureManager::LoadTintedTexture("sprites/characters/cutted-character/walking_sprites/walking_1.png", "sprites/characters/npc_walkA_tinted", 60, 110, 220, 0.45f);
    TextureManager::LoadTintedTexture("sprites/characters/cutted-character/walking_sprites/walking_4.png", "sprites/characters/npc_walkB_tinted", 60, 110, 220, 0.45f);
    anim.addAnimation("Idle", { "sprites/characters/npc_idle_tinted" }, 400);
    anim.addAnimation("Walk", { "sprites/characters/npc_walkA_tinted", "sprites/characters/npc_walkB_tinted" }, 250);
    anim.play("Idle");

    e.addComponent<CombatComponent>(2.0f, "npc");
    e.addComponent<NPCAIComponent>();
    return e;
}

// A world item icon (key/door/chest/boots), drawn stretched to fill one
// full tile like Java's object.draw() always does regardless of native
// pixel size (these source images are all 16x16).
static Entity& spawnItem(Manager& mgr, const char* path, float x, float y) {
    auto& e = mgr.addEntity();
    e.addComponent<PositionComponent>(x, y, 16, 16, 6); // 16*6 = 96 = one tile
    e.addComponent<SpriteComponent>(path);
    return e;
}

void Game::spawnWorldEntities() {
    const float t = 96.0f; // tile size, matches Map::TILE_SIZE / Java's tileSize

    // Same world layout Java's Asset.setCharacters()/setObject() uses - both
    // games now share the same 50x50 map and 96px tile size, so the tile
    // coordinates translate directly.
    spawnEnemy(manager, 10 * t, 10 * t);
    spawnEnemy(manager, 35 * t, 15 * t);
    spawnEnemy(manager, 15 * t, 35 * t);
    spawnEnemy(manager, 40 * t, 40 * t);
    spawnEnemy(manager, 25 * t, 12 * t);
    spawnEnemy(manager, 8 * t, 30 * t);

    spawnNPC(manager, 20 * t, 20 * t);
    spawnNPC(manager, 30 * t, 30 * t);
    spawnNPC(manager, 12 * t, 40 * t);

    auto& key = spawnItem(manager, "sprites/objects/pickups/key.png", 23 * t, 7 * t);
    key.addComponent<PickupComponent>(PickupComponent::Type::Key);

    // Chest and boots are decorative placeholders in the original Java game
    // too (no pickup case is wired up for them there either) - ported as-is.
    spawnItem(manager, "sprites/objects/pickups/chest.png", 23 * t, 8 * t);
    spawnItem(manager, "sprites/objects/pickups/boots.png", 23 * t, 10 * t);

    auto& door = spawnItem(manager, "sprites/objects/pickups/door.png", 23 * t, 9 * t);
    door.addComponent<ColliderComponent>("door"); // solid until opened
    door.addComponent<PickupComponent>(PickupComponent::Type::Door, 24.0f);
}

void Game::selectCharacter(int choice) {
    if (Game::gameStarted) return;

    std::string folder = (choice == 2) ? "cutted-character-girl" : "cutted-character";
    if (player.hasComponent<AnimationComponent>()) {
        loadPlayerAnimations(player.getComponent<AnimationComponent>(), folder);
        player.getComponent<AnimationComponent>().reset("Idle", true);
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Character selected: %s", folder.c_str());
    Game::gameStarted = true;
}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen) {
    Uint32 flags = 0;
    if(fullscreen) flags = SDL_WINDOW_FULLSCREEN;

    // Determine actual window size (on Android the provided width/height may be 0)
    int actualW = width;
    int actualH = height;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        // Set log priority to ensure all INFO messages appear in logcat
        SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
        SDL_Log("=== GAME INIT START ===");
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init succeeded");

        #ifdef __ANDROID__
            // Use the OpenGL ES renderer on Android. (Pixel-art nearest
            // filtering is set per texture by TextureManager - SDL3 has no
            // global SDL2-style RENDER_SCALE_QUALITY hint.)
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
            SDL_Log("[ANDROID] Set OpenGL ES renderer hint");
        #endif

        window = SDL_CreateWindow(title, width, height, flags);
        if(window) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window created successfully");
        }

        renderer = SDL_CreateRenderer(window, NULL);
        if(renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer created successfully");
        }

        // Update actual window size if needed
        if (actualW == 0 || actualH == 0) {
            SDL_GetWindowSize(window, &actualW, &actualH);
            if (actualW == 0 || actualH == 0) {
                // If we still don't have a size, use a sensible default
                actualW = 1280;
                actualH = 720;
            }
        }

        // Show a simple loading screen so the window appears responsive while textures load
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_FRect box{ (float)(actualW/2 - 120), (float)(actualH/2 - 20), 240.0f, 40.0f };
            SDL_RenderFillRect(renderer, &box);
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            SDL_FRect inner{ (float)(actualW/2 - 110), (float)(actualH/2 - 10), 220.0f, 20.0f };
            SDL_RenderFillRect(renderer, &inner);
            SDL_RenderPresent(renderer);
            SDL_Delay(50);
            SDL_PumpEvents();
        }

        srand((unsigned int)SDL_GetTicks());
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished initial window/renderer setup");
        // record screen size for touch mapping (use actual sizes)
        Game::screenWidth = actualW;
        Game::screenHeight = actualH;
        // Try to initialize haptic (optional)
        if (SDL_InitSubSystem(SDL_INIT_HAPTIC) == 0) {
            Game::haptic = SDL_OpenHaptic(0);
            if (Game::haptic) {
                if (SDL_InitHapticRumble(Game::haptic) != 0) {
                    SDL_CloseHaptic(Game::haptic);
                    Game::haptic = nullptr;
                }
            }
        }
        isRunning = true;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        isRunning = false;
    }

    map = new Map();
    camera = new Camera(actualW, actualH);
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

    // ECS: create player
    player.addComponent<PositionComponent>(2400.0f, 2400.0f, 48, 48, 3);
    AnimationComponent& playerAnim = player.addComponent<AnimationComponent>();
    loadPlayerAnimations(playerAnim, "cutted-character"); // boy by default; character-select may reload as girl
    playerAnim.play("Idle");

    player.addComponent<Keyboard>();
    player.addComponent<MouseHandler>();
    player.addComponent<AttackComponent>(); // added after Keyboard so it reads this frame's velocity for facing
    player.addComponent<InventoryComponent>();
    player.addComponent<ColliderComponent>("player");
    const float playerMaxHp = 5.0f;
    player.addComponent<CombatComponent>(playerMaxHp, "player");
    this->hud->init((int)playerMaxHp);
    this->hud->bindCombatComponent(&player.getComponent<CombatComponent>());

    Game::playerEntity = &player;

    previewBoy = TextureManager::LoadTexture("sprites/characters/cutted-character/standing_sprites/standing_1.png");
    previewGirl = TextureManager::LoadTexture("sprites/characters/cutted-character-girl/standing_sprites/standing_1.png");

    spawnWorldEntities();

    Game::gameStarted = false;
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
                if (!Game::gameStarted && event.key.repeat == 0) {
                    if (event.key.key == SDLK_1) selectCharacter(1);
                    else if (event.key.key == SDLK_2) selectCharacter(2);
                }
                break;
            case SDL_EVENT_FINGER_DOWN: {
                long long fid = (long long)event.tfinger.fingerID;
                float fx = event.tfinger.x * (float)Game::screenWidth;
                float fy = event.tfinger.y * (float)Game::screenHeight;
                if (!Game::gameStarted) {
                    // Character select on touch screens: boy is drawn on the left, girl on the right
                    selectCharacter(fx < Game::screenWidth / 2.0f ? 1 : 2);
                    break;
                }
                if (fx < (Game::screenWidth / 2) && Game::movementFingerId == -1) {
                    Game::movementFingerId = fid;
                    Game::movementActive = true;
                    // compute movementDX/DY relative to joystick center (match visual at 5% + half width)
                    float jbW = 160.0f;
                    float cx = (Game::screenWidth * 0.05f) + jbW / 2.0f;
                    float cy = (Game::screenHeight * 0.65f) + jbW / 2.0f;
                    float dx = fx - cx;
                    float dy = fy - cy;
                    float maxr = 64.0f;
                    Game::movementDX = fmaxf(-1.0f, fminf(1.0f, dx / maxr));
                    Game::movementDY = fmaxf(-1.0f, fminf(1.0f, dy / maxr));
                } else if (fx >= (Game::screenWidth / 2) && Game::attackFingerId == -1) {
                    Game::attackFingerId = fid;
                    Game::attackActive = true;
                }
                break; }
            case SDL_EVENT_FINGER_UP: {
                long long fid = (long long)event.tfinger.fingerID;
                if (fid == Game::movementFingerId) {
                    Game::movementFingerId = -1;
                    Game::movementActive = false;
                    Game::movementDX = 0.0f; Game::movementDY = 0.0f;
                }
                if (fid == Game::attackFingerId) {
                    Game::attackFingerId = -1;
                    Game::attackActive = false;
                }
                break; }
            case SDL_EVENT_FINGER_MOTION: {
                long long fid = (long long)event.tfinger.fingerID;
                float fx = event.tfinger.x * (float)Game::screenWidth;
                float fy = event.tfinger.y * (float)Game::screenHeight;
                if (fid == Game::movementFingerId) {
                    float jbW = 160.0f;
                    float cx = (Game::screenWidth * 0.05f) + jbW / 2.0f;
                    float cy = (Game::screenHeight * 0.65f) + jbW / 2.0f;
                    float dx = fx - cx;
                    float dy = fy - cy;
                    float maxr = 64.0f;
                    Game::movementDX = fmaxf(-1.0f, fminf(1.0f, dx / maxr));
                    Game::movementDY = fmaxf(-1.0f, fminf(1.0f, dy / maxr));
                    Game::movementActive = true;
                }
                if (fid == Game::attackFingerId) {
                    Game::attackActive = true;
                }
                break; }

            default:
                break;
        }
    }
}

void Game::update() {
    if (!Game::gameStarted) return; // frozen on the character-select screen

    manager.refresh();
    manager.update();

    if (player.hasComponent<PositionComponent>()) {
        Vector2D playerPos = player.getComponent<PositionComponent>().position;
        camera->update(playerPos);
    }
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

void Game::renderCharacterSelect() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 24, 26, 38, 255);
    SDL_FRect bg{ 0, 0, (float)Game::screenWidth, (float)Game::screenHeight };
    SDL_RenderFillRect(renderer, &bg);

    float iconSize = 160.0f;
    float gap = 120.0f;
    float boyX = Game::screenWidth / 2.0f - gap / 2.0f - iconSize;
    float girlX = Game::screenWidth / 2.0f + gap / 2.0f;
    float y = Game::screenHeight / 2.0f - iconSize / 2.0f - 30.0f;

    auto drawOption = [&](SDL_Texture* tex, float x, int digit) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 40);
        SDL_FRect panel{ x - 14, y - 14, iconSize + 28, iconSize + 28 };
        SDL_RenderFillRect(renderer, &panel);

        if (tex) {
            float tw = 0.0f, th = 0.0f;
            SDL_GetTextureSize(tex, &tw, &th);
            SDL_FRect src{ 0, 0, tw, th };
            SDL_FRect dest{ x, y, iconSize, iconSize };
            TextureManager::Draw(tex, src, dest);
        }

        SDL_Color white{ 255, 255, 255, 255 };
        SevenSegment::drawDigit(renderer, digit, x + iconSize / 2.0f - 20.0f, y + iconSize + 24.0f, 40.0f, 56.0f, white);
    };

    drawOption(previewBoy, boyX, 1);
    drawOption(previewGirl, girlX, 2);
}

void Game::render() {
    // SDL_RenderClear uses the current draw color, which would otherwise be
    // whatever the previous frame drew last (e.g. the red Android attack button).
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (!Game::gameStarted) {
        renderCharacterSelect();
        SDL_RenderPresent(renderer);
        return;
    }

    map->DrawMap(camera);
    manager.draw();

    if (player.hasComponent<CombatComponent>() && player.getComponent<CombatComponent>().isDead) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 160, 0, 0, 90);
        SDL_FRect overlay{ 0, 0, (float)Game::screenWidth, (float)Game::screenHeight };
        SDL_RenderFillRect(renderer, &overlay);
    }

    if (this->hud) this->hud->render();
    // Draw simple on-screen controls for Android
#ifdef __ANDROID__
    // semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    // Joystick base (bottom-left) - use float rects for SDL3
    float jbW = 160.0f, jbH = 160.0f;
    float jbX = Game::screenWidth * 0.05f;
    float jbY = Game::screenHeight * 0.65f;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 96);
    SDL_FRect jb{jbX, jbY, jbW, jbH};
    SDL_RenderFillRect(renderer, &jb);
    // Joystick knob
    float cx = jbX + jbW/2.0f;
    float cy = jbY + jbH/2.0f;
    float radius = 48.0f;
    float kx = cx + (Game::movementDX * (jbW/2.0f - radius));
    float ky = cy + (Game::movementDY * (jbH/2.0f - radius));
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    SDL_FRect knob{ kx - radius/2.0f, ky - radius/2.0f, radius, radius };
    SDL_RenderFillRect(renderer, &knob);

    // Attack button (bottom-right)
    float abW = 120.0f, abH = 120.0f;
    float abX = (float)Game::screenWidth - (Game::screenWidth * 0.05f) - abW;
    float abY = Game::screenHeight * 0.70f;
    SDL_SetRenderDrawColor(renderer, 180, 30, 30, Game::attackActive ? 220 : 120);
    SDL_FRect ab{ abX, abY, abW, abH };
    SDL_RenderFillRect(renderer, &ab);
#endif
    SDL_RenderPresent(renderer);
}
