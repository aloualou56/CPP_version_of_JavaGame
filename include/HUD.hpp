#ifndef HUD_hpp
#define HUD_hpp

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include "TextureManager.hpp"

class HUD {
public:
    HUD();
    ~HUD();

    // Initialize with desired max health and attempt to load textures
    void init(int maxHealth);
    void setHealth(float current, int max);
    // Bind to ECS CombatComponent so HUD reads health directly
    void bindCombatComponent(class CombatComponent* cc);
    void render();

private:
    SDL_Texture* heartFull = nullptr;
    SDL_Texture* heart3q = nullptr;
    SDL_Texture* heartHalf = nullptr;
    SDL_Texture* heart1q = nullptr;
    SDL_Texture* heartEmpty = nullptr;
    bool texturesLoaded = false;
    float currentHealth = 0.0f;
    int maxHealth = 0;
    // Optional pointer to an ECS CombatComponent to read health from
    class CombatComponent* healthComp = nullptr;
    int heartSize = 48; // px (2x bigger)
    int padding = 6; // spacing between hearts
    int offsetX = 16; // top-left corner offset
    int offsetY = 12;
};

#endif
