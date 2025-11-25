#include <TextureManager.hpp>
#include <unordered_map>
#include <cctype>
#include <atomic>
#include <SDL.h>
#include <SDL_image.h>
#include <map>
#include <string>
#include <mutex>

// Progress counters for loading screen
static std::atomic<int> g_totalToLoad{0};
static std::atomic<int> g_loadedCount{0};

// Simple texture cache owned by TextureManager. Keys are the file-paths
// or arbitrary strings supplied by RegisterTexture.
static std::map<std::string, SDL_Texture*> g_textureCache;
static std::mutex g_cacheMutex;

SDL_Texture* TextureManager::LoadTexture(const char* texture) {

    std::string key = texture ? std::string(texture) : std::string();
    // If cached, return immediately
    {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        auto it = g_textureCache.find(key);
        if (it != g_textureCache.end()) {
            // Update progress counters (if enabled) for compatibility with previous behavior
            if (g_totalToLoad > 0) {
                ++g_loadedCount;
                if (Game::renderer) {
                    int total = static_cast<int>(g_totalToLoad.load());
                    int loaded = static_cast<int>(g_loadedCount.load());
                    float pct = total > 0 ? (float)loaded / (float)total : 1.0f;
                    if (pct < 0.0f) pct = 0.0f;
                    if (pct > 1.0f) pct = 1.0f;
                    int w = 240, h = 20;
                    int winW = 800, winH = 600;
                    SDL_GetRendererOutputSize(Game::renderer, &winW, &winH);
                    SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);
                    SDL_SetRenderDrawColor(Game::renderer, 20, 20, 20, 255);
                    SDL_RenderClear(Game::renderer);
                    SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, 255);
                    SDL_Rect box{ winW/2 - w/2, winH/2 - h/2, w, h };
                    SDL_RenderFillRect(Game::renderer, &box);
                    SDL_SetRenderDrawColor(Game::renderer, 40, 40, 40, 255);
                    SDL_Rect inner{ winW/2 - w/2 + 5, winH/2 - h/2 + 4, w - 10, h - 8 };
                    SDL_RenderFillRect(Game::renderer, &inner);
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
                int totalNow = static_cast<int>(g_totalToLoad.load());
                int loadedNow = static_cast<int>(g_loadedCount.load());
                if (totalNow > 0 && loadedNow >= totalNow) {
                    g_totalToLoad = 0;
                    g_loadedCount = 0;
                }
            }
            return it->second;
        }
    }

    // Load surface and create texture. First try IMG_Load (works on desktop
    // and many SDL_image Android builds). If that fails (e.g. assets are
    // packaged inside the APK), try SDL_RWFromFile + IMG_Load_RW which can
    // read packaged assets via SDL's RW API.
    SDL_Surface* tempSurface = IMG_Load(texture);
    if (!tempSurface) {
        // Log why IMG_Load failed for debugging
        const char* imgErr = IMG_GetError();
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load failed for '%s' -> %s", texture ? texture : "(null)", imgErr ? imgErr : "(no error)");
        SDL_RWops* rw = SDL_RWFromFile(texture, "rb");
        if (rw) {
            tempSurface = IMG_Load_RW(rw, 1); // auto-free rw
            if (!tempSurface) {
                const char* imgErr2 = IMG_GetError();
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load_RW failed for '%s' -> %s", texture ? texture : "(null)", imgErr2 ? imgErr2 : "(no error)");
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_RWFromFile returned NULL for '%s'", texture ? texture : "(null)");
        }
    }
    SDL_Texture* tex = nullptr;
    if (tempSurface) {
        tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
        SDL_FreeSurface(tempSurface);
    }
    else {
        // Log failure to create a surface for diagnosing missing/corrupt assets
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image '%s' (texture will be null)", texture ? texture : "(null)");
    }

    // Insert into cache if valid
    if (tex) {
        std::lock_guard<std::mutex> lk(g_cacheMutex);
        g_textureCache[key] = tex;
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

void TextureManager::RegisterTexture(const char* key, SDL_Texture* tex) {
    if (!key || !tex) return;
    std::lock_guard<std::mutex> lk(g_cacheMutex);
    std::string k(key);
    auto it = g_textureCache.find(k);
    if (it == g_textureCache.end()) {
        g_textureCache[k] = tex;
    }
}

SDL_Texture* TextureManager::GetTexture(const char* key) {
    if (!key) return nullptr;
    std::lock_guard<std::mutex> lk(g_cacheMutex);
    auto it = g_textureCache.find(std::string(key));
    if (it == g_textureCache.end()) return nullptr;
    return it->second;
}

void TextureManager::ClearCache() {
    std::lock_guard<std::mutex> lk(g_cacheMutex);
    for (auto &p : g_textureCache) {
        if (p.second) SDL_DestroyTexture(p.second);
    }
    g_textureCache.clear();
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
    // Similar fallback for DetectBottomOpaqueRow: try IMG_Load first, then
    // SDL_RWFromFile/IMG_Load_RW for packaged assets on Android.
    SDL_Surface* surf = IMG_Load(fileName);
    if (!surf) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load failed for '%s' in DetectBottomOpaqueRow: %s", fileName ? fileName : "(null)", IMG_GetError());
        SDL_RWops* rw = SDL_RWFromFile(fileName, "rb");
        if (rw) {
            surf = IMG_Load_RW(rw, 1);
            if (!surf) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load_RW failed for '%s' in DetectBottomOpaqueRow: %s", fileName ? fileName : "(null)", IMG_GetError());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_RWFromFile returned NULL for '%s' in DetectBottomOpaqueRow", fileName ? fileName : "(null)");
        }
    }
    if (!surf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DetectBottomOpaqueRow: could not load surface for '%s'", fileName ? fileName : "(null)");
        return -1;
    }

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
        // Try to read via SDL_RWFromFile first (works for packaged APK assets),
        // fall back to fopen for desktop where assets may be real files.
        SDL_RWops* rw = SDL_RWFromFile(path, "r");
        if (rw) {
            Sint64 sz = SDL_RWsize(rw);
            if (sz > 0) {
                std::string buf;
                buf.resize((size_t)sz);
                SDL_RWread(rw, &buf[0], 1, (size_t)sz);
                SDL_RWclose(rw);
                std::istringstream iss(buf);
                std::string line;
                while (std::getline(iss, line)) {
                    // Trim leading whitespace
                    size_t i = 0; while (i < line.size() && isspace((unsigned char)line[i])) i++;
                    if (i >= line.size()) continue;
                    if (line[i] == '#' ) continue;
                    std::string token = line.substr(i);
                    char img[384]; int val = -1;
                    if (sscanf(token.c_str(), "%383s %d", img, &val) == 2) {
                        overrides[std::string(img)] = val;
                    }
                }
            } else {
                SDL_RWclose(rw);
            }
        } else {
            FILE *f = fopen(path, "r");
            if (f) {
                char line[512];
                while (fgets(line, sizeof(line), f)) {
                    char *s = line;
                    while (*s && isspace((unsigned char)*s)) s++;
                    if (*s == '\0' || *s == '#' || *s == '\n') continue;
                    char img[384]; int val = -1;
                    if (sscanf(s, "%383s %d", img, &val) == 2) {
                        overrides[std::string(img)] = val;
                    }
                }
                fclose(f);
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Anchor overrides file not found: %s", path);
            }
        }
    }
    if (!fileName) return -1;
    auto it = overrides.find(std::string(fileName));
    if (it == overrides.end()) return -1;
    return it->second;
}