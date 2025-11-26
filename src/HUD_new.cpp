#include <HUD.hpp>
#include <Game.hpp>
#include <TextureManager.hpp>
#include <ECS/HealthComponent.hpp>
#include <filesystem>
#include <vector>
#include <cmath>

#include "stb_image_write.h"

HUD::HUD() {}

HUD::~HUD() {
    // Textures are owned by TextureManager cache. Do not free here.
}

static SDL_Texture* createTextureFromRGBA(int w, int h, const std::vector<unsigned char>& buf) {
    SDL_Texture* t = SDL_CreateTexture(Game::renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (!t) return nullptr;
    SDL_UpdateTexture(t, nullptr, buf.data(), w * 4);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

void HUD::init(int maxH) {
    maxHealth = maxH;
    currentHealth = (float)maxH;

    std::filesystem::create_directories("sprites/ui");

    std::string basePath = "sprites/ui/";
    std::string names[5] = {"heart_full.png","heart_3q.png","heart_half.png","heart_1q.png","heart_empty.png"};

    heartFull  = TextureManager::LoadTexture((basePath + names[0]).c_str());
    heart3q    = TextureManager::LoadTexture((basePath + names[1]).c_str());
    heartHalf  = TextureManager::LoadTexture((basePath + names[2]).c_str());
    heart1q    = TextureManager::LoadTexture((basePath + names[3]).c_str());
    heartEmpty = TextureManager::LoadTexture((basePath + names[4]).c_str());

    if (heartFull && heart3q && heartHalf && heart1q && heartEmpty) {
        texturesLoaded = true;
        float fw = 0;
        SDL_GetTextureSize(heartFull, &fw, nullptr);
        int w = (int)fw;
        if (w>0) heartSize = w;
        return;
    }

    // For any missing textures, we'll procedurally generate them below. Do not destroy
    // textures that were successfully loaded from disk; TextureManager owns them.

    const int patternW = 9;
    const int patternH = 8;
    static const int mask[patternH][patternW] = {
        {0,1,1,0,0,1,1,0,0},
        {1,1,1,1,1,1,1,1,0},
        {1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,0,0},
        {0,0,1,1,1,1,0,0,0},
        {0,0,0,1,1,0,0,0,0},
        {0,0,0,0,1,0,0,0,0},
        {0,0,0,0,0,0,0,0,0}
    };

    int w = heartSize, h = heartSize;
    int scale = heartSize / patternW;
    if (scale < 1) scale = 1;
    int marginX = (heartSize - (patternW * scale)) / 2;
    int marginY = (heartSize - (patternH * scale)) / 2;

    auto gen_buffer = [&](int quarters)->std::vector<unsigned char> {
        std::vector<unsigned char> buf(w * h * 4);
        std::fill(buf.begin(), buf.end(), 0);

        for (int py = 0; py < patternH; ++py) {
            for (int px = 0; px < patternW; ++px) {
                if (!mask[py][px]) continue;
                for (int dy = 0; dy < scale; ++dy) {
                    for (int dx = 0; dx < scale; ++dx) {
                        int cx = marginX + px*scale + dx;
                        int cy = marginY + py*scale + dy;
                        if (cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
                        int idx = (cy * w + cx) * 4;

                        unsigned char fullR = 200, fullG = 30, fullB = 45;
                        unsigned char emptyR = 70, emptyG = 70, emptyB = 70;

                        bool drawFull = (quarters == 4);
                        if (drawFull) {
                            buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255;
                        } else if (quarters == 3) {
                            if (dx < (scale*3)/4) { buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255; }
                            else { buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255; }
                        } else if (quarters == 2) {
                            if (dx < scale/2) { buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255; }
                            else { buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255; }
                        } else if (quarters == 1) {
                            if (dx < scale/4) { buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255; }
                            else { buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255; }
                        } else {
                            buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255;
                        }
                    }
                }
            }
        }
        return buf;
    };

    std::vector<unsigned char> bufFull = gen_buffer(4);
    std::vector<unsigned char> buf3q  = gen_buffer(3);
    std::vector<unsigned char> bufHalf= gen_buffer(2);
    std::vector<unsigned char> buf1q  = gen_buffer(1);
    std::vector<unsigned char> bufE   = gen_buffer(0);

    // Create textures for any that are missing and register them with TextureManager
    if (!heartFull)  heartFull  = createTextureFromRGBA(w,h,bufFull);
    if (!heart3q)    heart3q    = createTextureFromRGBA(w,h,buf3q);
    if (!heartHalf)  heartHalf  = createTextureFromRGBA(w,h,bufHalf);
    if (!heart1q)    heart1q    = createTextureFromRGBA(w,h,buf1q);
    if (!heartEmpty) heartEmpty = createTextureFromRGBA(w,h,bufE);

    // Write files to disk (so future runs can load them) and register generated textures
    stbi_write_png((basePath + names[0]).c_str(), w, h, 4, bufFull.data(), w*4);
    stbi_write_png((basePath + names[1]).c_str(), w, h, 4, buf3q.data(), w*4);
    stbi_write_png((basePath + names[2]).c_str(), w, h, 4, bufHalf.data(), w*4);
    stbi_write_png((basePath + names[3]).c_str(), w, h, 4, buf1q.data(), w*4);
    stbi_write_png((basePath + names[4]).c_str(), w, h, 4, bufE.data(), w*4);

    // Register textures in TextureManager so they are owned and freed centrally
    TextureManager::RegisterTexture((basePath + names[0]).c_str(), heartFull);
    TextureManager::RegisterTexture((basePath + names[1]).c_str(), heart3q);
    TextureManager::RegisterTexture((basePath + names[2]).c_str(), heartHalf);
    TextureManager::RegisterTexture((basePath + names[3]).c_str(), heart1q);
    TextureManager::RegisterTexture((basePath + names[4]).c_str(), heartEmpty);

    stbi_write_png((basePath + names[0]).c_str(), w, h, 4, bufFull.data(), w*4);
    stbi_write_png((basePath + names[1]).c_str(), w, h, 4, buf3q.data(), w*4);
    stbi_write_png((basePath + names[2]).c_str(), w, h, 4, bufHalf.data(), w*4);
    stbi_write_png((basePath + names[3]).c_str(), w, h, 4, buf1q.data(), w*4);
    stbi_write_png((basePath + names[4]).c_str(), w, h, 4, bufE.data(), w*4);

    texturesLoaded = true;
}

void HUD::setHealth(float current, int max) {
    currentHealth = current;
    maxHealth = max;
}

void HUD::bindHealthComponent(HealthComponent* hc) {
    healthComp = hc;
    if (healthComp) {
        maxHealth = healthComp->getMax();
        currentHealth = healthComp->getCurrent();
    }
}

void HUD::render() {
    if (maxHealth <= 0) return;

    for (int i = 0; i < maxHealth; ++i) {
        int x = offsetX + i * (heartSize + padding);
        int y = offsetY;
        SDL_Rect dest = { x, y, heartSize, heartSize };

        // Prefer the bound ECS health component, if provided
        float useHealth = currentHealth;
        if (healthComp) useHealth = healthComp->getCurrent();
        float remain = useHealth - (float)i;
        int quarters = 0;
        if (remain >= 1.0f) quarters = 4;
        else if (remain <= 0.0f) quarters = 0;
        else {
            quarters = (int)ceilf(remain * 4.0f - 0.001f);
            if (quarters < 0) quarters = 0;
            if (quarters > 4) quarters = 4;
        }

        if (texturesLoaded) {
            SDL_Rect src = {0,0,heartSize,heartSize};
            switch (quarters) {
                case 4: TextureManager::Draw(heartFull, src, dest); break;
                case 3: TextureManager::Draw(heart3q, src, dest); break;
                case 2: TextureManager::Draw(heartHalf, src, dest); break;
                case 1: TextureManager::Draw(heart1q, src, dest); break;
                default: TextureManager::Draw(heartEmpty, src, dest); break;
            }
        } else {
            SDL_FRect destF = { (float)dest.x, (float)dest.y, (float)dest.w, (float)dest.h };
            if (remain >= 1.0f) SDL_SetRenderDrawColor(Game::renderer, 200,30,45,255);
            else if (remain > 0.0f) SDL_SetRenderDrawColor(Game::renderer, 150,40,50,255);
            else SDL_SetRenderDrawColor(Game::renderer, 80,80,80,255);
            SDL_RenderFillRect(Game::renderer, &destF);
            SDL_SetRenderDrawColor(Game::renderer, 0,0,0,255);
            SDL_RenderRect(Game::renderer, &destF);
        }
    }
}
