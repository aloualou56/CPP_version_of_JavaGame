#include "TextureManager.hpp"
#include <unordered_map>
#include <cctype>
#include <atomic>
#include <SDL.h>

// Progress counters for loading screen
static std::atomic<int> g_totalToLoad{0};
static std::atomic<int> g_loadedCount{0};

SDL_Texture* TextureManager::LoadTexture(const char* texture) {

    // Load surface and create texture
    SDL_Surface* tempSurface = IMG_Load(texture);
    SDL_Texture* tex = nullptr;
    if (tempSurface) {
        tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
        SDL_FreeSurface(tempSurface);
    }

    // Update progress and draw a progress bar if we have a target
    if (g_totalToLoad > 0) {
        ++g_loadedCount;
        if (Game::renderer) {
            int total = static_cast<int>(g_totalToLoad.load());
            int loaded = static_cast<int>(g_loadedCount.load());
            float pct = total > 0 ? (float)loaded / (float)total : 1.0f;
            if (pct < 0.0f) pct = 0.0f;
            if (pct > 1.0f) pct = 1.0f;

            // Draw simple progress bar centered in the window
            int w = 240, h = 20;
            int winW = 800, winH = 600;
            // try to query actual renderer output size
            if (Game::renderer) {
                SDL_GetRendererOutputSize(Game::renderer, &winW, &winH);
            }

            SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);
            // background
            SDL_SetRenderDrawColor(Game::renderer, 20, 20, 20, 255);
            SDL_RenderClear(Game::renderer);
            // outer box
            SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, 255);
            SDL_Rect box{ winW/2 - w/2, winH/2 - h/2, w, h };
            SDL_RenderFillRect(Game::renderer, &box);
            // inner
            SDL_SetRenderDrawColor(Game::renderer, 40, 40, 40, 255);
            SDL_Rect inner{ winW/2 - w/2 + 5, winH/2 - h/2 + 4, w - 10, h - 8 };
            SDL_RenderFillRect(Game::renderer, &inner);
            // progress (clamp width)
            SDL_SetRenderDrawColor(Game::renderer, 100, 220, 100, 255);
            int pw = static_cast<int>((w - 10) * pct);
            if (pw < 0) pw = 0;
            if (pw > inner.w) pw = inner.w;
            SDL_Rect prog{ inner.x, inner.y, pw, inner.h };
            SDL_RenderFillRect(Game::renderer, &prog);
            SDL_RenderPresent(Game::renderer);
            SDL_PumpEvents();
            SDL_Delay(8);
        }

        // If we've reached or exceeded the expected total, disable progress mode so later runtime
        // loads don't show the loading UI. This prevents the loading bar from appearing during gameplay.
        int totalNow = static_cast<int>(g_totalToLoad.load());
        int loadedNow = static_cast<int>(g_loadedCount.load());
        if (totalNow > 0 && loadedNow >= totalNow) {
            g_totalToLoad = 0;
            g_loadedCount = 0;
        }
    }

    return tex;
}

void TextureManager::SetTotalToLoad(int total) {
    g_totalToLoad = total;
    g_loadedCount = 0;
}

int TextureManager::GetLoadedCount() { return static_cast<int>(g_loadedCount.load()); }

int TextureManager::GetTotalToLoad() { return static_cast<int>(g_totalToLoad.load()); }

// Συμβατότητα προς τα πίσω: υπερφόρτωση 3 ορισμάτων (κάποια μεταγλωττισμένα αντικείμενα μπορεί
// να αναφέρονται σε αυτό το σύμβολο)
void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest) {
    TextureManager::Draw(tex, src, dest, SDL_FLIP_NONE);
}

void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip) {
    // Χρήση SDL_RenderCopyEx για να επιτρέπεται ο οριζόντιος κατοπτρισμός (flip) του χαρακτήρα
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0.0, nullptr, flip);
}

int TextureManager::DetectBottomOpaqueRow(const char* fileName) {
    SDL_Surface* surf = IMG_Load(fileName);
    if (!surf) return -1;

    SDL_PixelFormat *fmt = surf->format;
    int bpp = fmt->BytesPerPixel;

    if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);

    // Use a stronger alpha threshold and require a minimum fraction of pixels
    // in the row to be opaque. This ignores faint semi-transparent shadows.
    const int ALPHA_THRESHOLD = 200; // 0-255
    const float MIN_FRACTION = 0.06f; // 6% of row pixels must be opaque

    for (int y = surf->h - 1; y >= 0; --y) {
        int opaqueCount = 0;
        for (int x = 0; x < surf->w; ++x) {
            Uint32 pixel = 0;
            Uint8 *p = (Uint8*)surf->pixels + y * surf->pitch + x * bpp;
            switch (bpp) {
                case 1: pixel = *p; break;
                case 2: pixel = *(Uint16*)p; break;
                case 3:
                    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                        pixel = p[0] << 16 | p[1] << 8 | p[2];
                    else
                        pixel = p[0] | p[1] << 8 | p[2] << 16;
                    break;
                case 4: pixel = *(Uint32*)p; break;
            }
            Uint8 r,g,b,a;
            SDL_GetRGBA(pixel, fmt, &r, &g, &b, &a);
            if (a >= ALPHA_THRESHOLD) opaqueCount++;
        }

        if (opaqueCount >= (int)(surf->w * MIN_FRACTION)) {
            if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);
            int result = y;
            SDL_FreeSurface(surf);
            return result;
        }
    }

    if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);
    int fallback = surf->h - 1;
    SDL_FreeSurface(surf);
    return fallback;
}

// Simple anchor override loader/lookup. File format (optional):
// assets/anchor_overrides.txt
// <relative-path-to-image> <anchor-row-from-top>
// lines starting with # are comments.
int TextureManager::GetAnchorOverride(const char* fileName) {
    static bool loaded = false;
    static std::unordered_map<std::string, int> overrides;
    if (!loaded) {
        loaded = true;
        const char *path = "assets/anchor_overrides.txt";
        FILE *f = fopen(path, "r");
        if (!f) return -1;
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            // Trim leading whitespace
            char *s = line;
            while (*s && isspace((unsigned char)*s)) s++;
            if (*s == '\0' || *s == '#' || *s == '\n') continue;
            // Parse token and int
            char img[384]; int val = -1;
            if (sscanf(s, "%383s %d", img, &val) == 2) {
                overrides[std::string(img)] = val;
            }
        }
        fclose(f);
    }
    if (!fileName) return -1;
    auto it = overrides.find(std::string(fileName));
    if (it == overrides.end()) return -1;
    return it->second;
}