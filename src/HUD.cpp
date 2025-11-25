// Clean HUD implementation: loads (or generates) 5 PNG heart assets and renders fractional hearts
#include <HUD.hpp>
#include <Game.hpp>
#include <TextureManager.hpp>
#include <filesystem>
#include <vector>
#include <cmath>

// include stb_image_write header here so we can save generated PNGs
#include "stb_image_write.h"

HUD::HUD() {}

HUD::~HUD() {
    // Textures are owned by TextureManager cache; do not destroy here.
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

    // Attempt to load all five variants
    heartFull  = TextureManager::LoadTexture((basePath + names[0]).c_str());
    heart3q    = TextureManager::LoadTexture((basePath + names[1]).c_str());
    heartHalf  = TextureManager::LoadTexture((basePath + names[2]).c_str());
    heart1q    = TextureManager::LoadTexture((basePath + names[3]).c_str());
    heartEmpty = TextureManager::LoadTexture((basePath + names[4]).c_str());

    if (heartFull && heart3q && heartHalf && heart1q && heartEmpty) {
        texturesLoaded = true;
        int w,h;
        SDL_QueryTexture(heartFull, nullptr, nullptr, &w, &h);
        if (w>0) heartSize = w;
        return;
    }

    // Do not free partially loaded textures here; TextureManager owns cached textures.

    // Generate pixel-art hearts in memory and save them as PNGs
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
        // clear transparent
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

                        // colors
                        unsigned char fullR = 200, fullG = 30, fullB = 45;
                        unsigned char emptyR = 70, emptyG = 70, emptyB = 70;

                        bool drawFull = (quarters == 4);
                        // For partials, we split horizontally left->right
                        if (drawFull) {
                            buf[idx+0] = fullB;
                            buf[idx+1] = fullG;
                            buf[idx+2] = fullR;
                            buf[idx+3] = 255;
                        } else if (quarters == 3) {
                            if (dx < (scale*3)/4) {
                                buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255;
                            } else {
                                buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255;
                            }
                        } else if (quarters == 2) {
                            if (dx < scale/2) {
                                buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255;
                            } else {
                                buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255;
                            }
                        } else if (quarters == 1) {
                            if (dx < scale/4) {
                                buf[idx+0] = fullB; buf[idx+1] = fullG; buf[idx+2] = fullR; buf[idx+3] = 255;
                            } else {
                                buf[idx+0] = emptyB; buf[idx+1] = emptyG; buf[idx+2] = emptyR; buf[idx+3] = 255;
                            }
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

    // Create any missing textures and register them in the TextureManager cache
    if (!heartFull)  heartFull  = createTextureFromRGBA(w,h,bufFull);
    if (!heart3q)    heart3q    = createTextureFromRGBA(w,h,buf3q);
    if (!heartHalf)  heartHalf  = createTextureFromRGBA(w,h,bufHalf);
    if (!heart1q)    heart1q    = createTextureFromRGBA(w,h,buf1q);
    if (!heartEmpty) heartEmpty = createTextureFromRGBA(w,h,bufE);

    // Save to PNG files so you have editable assets on disk
    stbi_write_png((basePath + names[0]).c_str(), w, h, 4, bufFull.data(), w*4);
    stbi_write_png((basePath + names[1]).c_str(), w, h, 4, buf3q.data(), w*4);
    stbi_write_png((basePath + names[2]).c_str(), w, h, 4, bufHalf.data(), w*4);
    stbi_write_png((basePath + names[3]).c_str(), w, h, 4, buf1q.data(), w*4);
    stbi_write_png((basePath + names[4]).c_str(), w, h, 4, bufE.data(), w*4);

    // Register generated textures so TextureManager owns them
    TextureManager::RegisterTexture((basePath + names[0]).c_str(), heartFull);
    TextureManager::RegisterTexture((basePath + names[1]).c_str(), heart3q);
    TextureManager::RegisterTexture((basePath + names[2]).c_str(), heartHalf);
    TextureManager::RegisterTexture((basePath + names[3]).c_str(), heart1q);
    TextureManager::RegisterTexture((basePath + names[4]).c_str(), heartEmpty);

    texturesLoaded = true;
}

void HUD::setHealth(float current, int max) {
    currentHealth = current;
    maxHealth = max;
}

void HUD::render() {
    if (maxHealth <= 0) return;

    for (int i = 0; i < maxHealth; ++i) {
        int x = offsetX + i * (heartSize + padding);
        int y = offsetY;
        SDL_Rect dest = { x, y, heartSize, heartSize };

        float remain = currentHealth - (float)i;
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
            if (remain >= 1.0f) SDL_SetRenderDrawColor(Game::renderer, 200,30,45,255);
            else if (remain > 0.0f) SDL_SetRenderDrawColor(Game::renderer, 150,40,50,255);
            else SDL_SetRenderDrawColor(Game::renderer, 80,80,80,255);
            SDL_RenderFillRect(Game::renderer, &dest);
            SDL_SetRenderDrawColor(Game::renderer, 0,0,0,255);
            SDL_RenderDrawRect(Game::renderer, &dest);
        }
    }
}
#include <HUD.hpp>
#include <Game.hpp>

HUD::HUD() {

}

HUD::~HUD() {
    // Textures owned by TextureManager; do not destroy here.
}

void HUD::init(int maxH) {
    maxHealth = maxH;
    currentHealth = maxH;
    // Try to load pixel-art heart textures; if missing, generate simple pixel-art hearts
    heartFull = TextureManager::LoadTexture("sprites/ui/heart_full.png");
    heartEmpty = TextureManager::LoadTexture("sprites/ui/heart_empty.png");

    if (heartFull && heartEmpty) {
        texturesLoaded = true;
        int w, h;
        SDL_QueryTexture(heartFull, nullptr, nullptr, &w, &h);
        if (w > 0 && h > 0) heartSize = h;
        return;
    }

    // Do not free partially loaded textures here; TextureManager owns cached textures.

    // Generate pixel-art heart textures into SDL_Textures so they match the game's aesthetic
    const int patternW = 9;
    const int patternH = 8;
    // simple heart mask (1 = filled pixel)
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

    // create textures with render target so we can draw into them
    SDL_Texture* genFull = SDL_CreateTexture(Game::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, heartSize, heartSize);
    SDL_Texture* genEmpty = SDL_CreateTexture(Game::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, heartSize, heartSize);
    #include <HUD.hpp>
    #include <Game.hpp>
    #include <filesystem>
    #include <vector>
    #include "stb_image_write.h"

    HUD::HUD() {}

    HUD::~HUD() {
        // Textures are managed by TextureManager; don't destroy here.
    }

    static bool file_exists(const std::string &p) {
        return std::filesystem::exists(p);
    }

    // Helper: create an SDL_Texture from raw RGBA buffer
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

        // Try to load all five textures
        heartFull = TextureManager::LoadTexture((basePath + names[0]).c_str());
        heart3q  = TextureManager::LoadTexture((basePath + names[1]).c_str());
        heartHalf= TextureManager::LoadTexture((basePath + names[2]).c_str());
        heart1q  = TextureManager::LoadTexture((basePath + names[3]).c_str());
        heartEmpty=TextureManager::LoadTexture((basePath + names[4]).c_str());

        if (heartFull && heart3q && heartHalf && heart1q && heartEmpty) {
            texturesLoaded = true;
            int w,h;
            SDL_QueryTexture(heartFull, nullptr, nullptr, &w, &h);
            if (w>0) heartSize = w;
            return;
        }

        // Do not destroy partially loaded textures here; TextureManager owns cached textures.

        // Generate pixel-art versions for each fraction and save to PNG files
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
            // clear transparent
            for (size_t i=0;i<buf.size();++i) buf[i]=0;
            for (int py=0; py<patternH; ++py) {
                for (int px=0; px<patternW; ++px) {
                    if (!mask[py][px]) continue;
                    // which part of this pixel is filled depending on quarters
                    // We'll treat quarters as how many sub-pixels per heart are filled overall.
                    // Simpler: for partial hearts, we draw full mask but reduce alpha/brightness to indicate partial filling,
                    // and overlay an empty-color portion for the unfilled quarters.
                    int baseR = 200, baseG = 30, baseB = 45;
                    int emptyR = 60, emptyG = 60, emptyB = 60;

                    // Determine if this heart is considered filled in this quarter level
                    // quarters: 0..4 where 4=full, 3=3/4, 2=1/2, 1=1/4, 0=empty
                    bool drawFilled = (quarters >= 4);
                    bool draw3q = (quarters == 3);
                    bool drawHalf = (quarters == 2);
                    bool draw1q = (quarters == 1);

                    for (int dy=0; dy<scale; ++dy) {
                        for (int dx=0; dx<scale; ++dx) {
                            int cx = marginX + px*scale + dx;
                            int cy = marginY + py*scale + dy;
                            int idx = (cy * w + cx) * 4;
                            if (drawFilled) {
                                buf[idx+0] = (unsigned char)baseB;
                                buf[idx+1] = (unsigned char)baseG;
                                buf[idx+2] = (unsigned char)baseR;
                                buf[idx+3] = 255;
                            } else if (draw3q) {
                                // leave rightmost quarter darker
                                if (dx < scale*3/4) {
                                    buf[idx+0] = (unsigned char)baseB;
                                    buf[idx+1] = (unsigned char)baseG;
                                    buf[idx+2] = (unsigned char)baseR;
                                    buf[idx+3] = 255;
                                } else {
                                    buf[idx+0] = (unsigned char)emptyB;
                                    buf[idx+1] = (unsigned char)emptyG;
                                    buf[idx+2] = (unsigned char)emptyR;
                                    buf[idx+3] = 255;
                                }
                            } else if (drawHalf) {
                                if (dx < scale/2) {
                                    buf[idx+0] = (unsigned char)baseB;
                                    buf[idx+1] = (unsigned char)baseG;
                                    buf[idx+2] = (unsigned char)baseR;
                                    buf[idx+3] = 255;
                                } else {
                                    buf[idx+0] = (unsigned char)emptyB;
                                    buf[idx+1] = (unsigned char)emptyG;
                                    buf[idx+2] = (unsigned char)emptyR;
                                    buf[idx+3] = 255;
                                }
                            } else if (draw1q) {
                                if (dx < scale/4) {
                                    buf[idx+0] = (unsigned char)baseB;
                                    buf[idx+1] = (unsigned char)baseG;
                                    buf[idx+2] = (unsigned char)baseR;
                                    buf[idx+3] = 255;
                                } else {
                                    buf[idx+0] = (unsigned char)emptyB;
                                    buf[idx+1] = (unsigned char)emptyG;
                                    buf[idx+2] = (unsigned char)emptyR;
                                    buf[idx+3] = 255;
                                }
                            } else {
                                // empty
                                buf[idx+0] = (unsigned char)emptyB;
                                buf[idx+1] = (unsigned char)emptyG;
                                buf[idx+2] = (unsigned char)emptyR;
                                buf[idx+3] = 255;
                            }
                        }
                    }
                }
            }
            return buf;
        };

        // generate for 4,3,2,1,0 quarters
        std::vector<unsigned char> bufFull = gen_buffer(4);
        std::vector<unsigned char> buf3q  = gen_buffer(3);
        std::vector<unsigned char> bufHalf= gen_buffer(2);
        std::vector<unsigned char> buf1q  = gen_buffer(1);
        std::vector<unsigned char> bufE   = gen_buffer(0);

        // create textures
        heartFull = createTextureFromRGBA(w,h,bufFull);
        heart3q  = createTextureFromRGBA(w,h,buf3q);
        heartHalf= createTextureFromRGBA(w,h,bufHalf);
        heart1q  = createTextureFromRGBA(w,h,buf1q);
        heartEmpty = createTextureFromRGBA(w,h,bufE);

        // save PNGs to disk so author can edit them later
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

    void HUD::render() {
        if (maxHealth <= 0) return;

        for (int i = 0; i < maxHealth; ++i) {
            int x = offsetX + i * (heartSize + padding);
            int y = offsetY;
            SDL_Rect dest = { x, y, heartSize, heartSize };

            float remain = currentHealth - (float)i; // e.g., 3.5 means heart index 0..2 full, index 3 half
            int quarters = 0;
            if (remain >= 1.0f) quarters = 4;
            else if (remain <= 0.0f) quarters = 0;
            else {
                quarters = (int)ceilf(remain * 4.0f - 0.001f); // map fractional part to 1..3
                if (quarters < 0) quarters = 0; if (quarters>4) quarters=4;
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
                // fallback simple rectangle
                if (remain >= 1.0f) SDL_SetRenderDrawColor(Game::renderer, 200,30,45,255);
                else if (remain > 0.0f) SDL_SetRenderDrawColor(Game::renderer, 150,40,50,255);
                else SDL_SetRenderDrawColor(Game::renderer, 80,80,80,255);
                SDL_RenderFillRect(Game::renderer, &dest);
                SDL_SetRenderDrawColor(Game::renderer, 0,0,0,255);
                SDL_RenderDrawRect(Game::renderer, &dest);
            }
        }
    }
}

void HUD::setHealth(int current, int max) {
    currentHealth = current;
    maxHealth = max;
}

void HUD::render() {
    if (maxHealth <= 0) return;

    for (int i = 0; i < maxHealth; ++i) {
        int x = offsetX + i * (heartSize + padding);
        int y = offsetY;

        SDL_Rect dest = { x, y, heartSize, heartSize };

        if (texturesLoaded) {
            SDL_Rect src = { 0, 0, heartSize, heartSize };
            if (i < currentHealth) {
                TextureManager::Draw(heartFull, src, dest);
            } else {
                TextureManager::Draw(heartEmpty, src, dest);
            }
        } else {
            // Fallback: draw a colored rectangle (red for full, dark gray for empty)
            if (i < currentHealth) {
                SDL_SetRenderDrawColor(Game::renderer, 200, 30, 45, 255);
            } else {
                SDL_SetRenderDrawColor(Game::renderer, 80, 80, 80, 255);
            }
            SDL_RenderFillRect(Game::renderer, &dest);
            // draw thin border
            SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(Game::renderer, &dest);
        }
    }
}
