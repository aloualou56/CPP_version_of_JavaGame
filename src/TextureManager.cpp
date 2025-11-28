#include <TextureManager.hpp>
#include <unordered_map>
#include <cctype>
#include <atomic>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <sstream>
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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager::LoadTexture called for key: %s", key.c_str());
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
                    SDL_GetRenderOutputSize(Game::renderer, &winW, &winH);
                    SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);
                    SDL_SetRenderDrawColor(Game::renderer, 20, 20, 20, 255);
                    SDL_RenderClear(Game::renderer);
                    SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, 255);
                    SDL_FRect box{ (float)(winW/2 - w/2), (float)(winH/2 - h/2), (float)w, (float)h };
                    SDL_RenderFillRect(Game::renderer, &box);
                    SDL_SetRenderDrawColor(Game::renderer, 40, 40, 40, 255);
                    SDL_FRect inner{ (float)(winW/2 - w/2 + 5), (float)(winH/2 - h/2 + 4), (float)(w - 10), (float)(h - 8) };
                    SDL_RenderFillRect(Game::renderer, &inner);
                    SDL_SetRenderDrawColor(Game::renderer, 100, 220, 100, 255);
                    int pw = static_cast<int>((w - 10) * pct);
                    if (pw < 0) pw = 0;
                    if (pw > inner.w) pw = (int)inner.w;
                    SDL_FRect prog{ inner.x, inner.y, (float)pw, inner.h };
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
    // packaged inside the APK), try SDL_IOFromFile + IMG_LoadIO which can
    // read packaged assets via SDL's IO API.
    SDL_Surface* tempSurface = IMG_Load(texture);
    if (tempSurface) {
        
    }
    if (!tempSurface) {
        // Log why IMG_Load failed for debugging
        const char* imgErr = SDL_GetError();
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load failed for '%s' -> %s", texture ? texture : "(null)", imgErr ? imgErr : "(no error)");
        SDL_IOStream* io = SDL_IOFromFile(texture, "rb");
        if (io) {
            tempSurface = IMG_Load_IO(io, true); // auto-close io
            if (!tempSurface) {
                const char* imgErr2 = SDL_GetError();
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_LoadIO failed for '%s' -> %s", texture ? texture : "(null)", imgErr2 ? imgErr2 : "(no error)");
            }
        #else
            SDL_Surface* conv = SDL_ConvertSurface(tempSurface, SDL_PIXELFORMAT_ABGR8888);
            if (conv) {
                tex = SDL_CreateTextureFromSurface(Game::renderer, conv);
                SDL_DestroySurface(conv);
            } else {
                tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
            }
            SDL_DestroySurface(tempSurface);
        #endif
        }
        else {
            // Log failure to create a surface for diagnosing missing/corrupt assets
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image '%s' (texture will be null)", texture ? texture : "(null)");
        }
                int h = conv->h;
                for (int y = 0; y < h; ++y) {
                    Uint8* row = pixels + y * pitch;
                    for (int x = 0; x < w; ++x) {
                        Uint8* px = row + x * 4;
                        Uint8 tmp = px[0]; // R
                        px[0] = px[1];     // G
                        px[1] = tmp;       // R
                        // px[2] = B, px[3] = A (unchanged)
                    }
                }
                tex = SDL_CreateTextureFromSurface(Game::renderer, conv);
                SDL_DestroySurface(conv);
            } else {
                tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
            }
            SDL_DestroySurface(tempSurface);
        #else
            SDL_Surface* conv = SDL_ConvertSurface(tempSurface, SDL_PIXELFORMAT_ABGR8888);
            if (conv) {
                tex = SDL_CreateTextureFromSurface(Game::renderer, conv);
                SDL_DestroySurface(conv);
            } else {
                tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
            }
            SDL_DestroySurface(tempSurface);
        #endif
    } else {
        // Log failure to create a surface for diagnosing missing/corrupt assets
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image '%s' (texture will be null)", texture ? texture : "(null)");
    }

    // Insert into cache if valid
    if (tex) {
        // For pixel-art assets, prefer nearest filtering to avoid blurring when scaled
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
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
                SDL_GetRenderOutputSize(Game::renderer, &winW, &winH);
            }

            SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);
            // background
            SDL_SetRenderDrawColor(Game::renderer, 20, 20, 20, 255);
            SDL_RenderClear(Game::renderer);
            // outer box
            SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, 255);
            SDL_FRect box{ (float)(winW/2 - w/2), (float)(winH/2 - h/2), (float)w, (float)h };
            SDL_RenderFillRect(Game::renderer, &box);
            // inner
            SDL_SetRenderDrawColor(Game::renderer, 40, 40, 40, 255);
            SDL_FRect inner{ (float)(winW/2 - w/2 + 5), (float)(winH/2 - h/2 + 4), (float)(w - 10), (float)(h - 8) };
            SDL_RenderFillRect(Game::renderer, &inner);
            // progress (clamp width)
            SDL_SetRenderDrawColor(Game::renderer, 100, 220, 100, 255);
            int pw = static_cast<int>((w - 10) * pct);
            if (pw < 0) pw = 0;
            if (pw > inner.w) pw = (int)inner.w;
            SDL_FRect prog{ inner.x, inner.y, (float)pw, inner.h };
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
        // Ensure consistent scale mode for registered textures too
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
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
void TextureManager::Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest) {
    TextureManager::Draw(tex, src, dest, SDL_FLIP_NONE);
}

void TextureManager::Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest, SDL_FlipMode flip) {
    // Skip rendering if texture is null (failed to load)
    if (!tex) {
        return;
    }
    // Χρήση SDL_RenderTextureRotated για να επιτρέπεται ο οριζόντιος κατοπτρισμός (flip) του χαρακτήρα
    SDL_RenderTextureRotated(Game::renderer, tex, &src, &dest, 0.0, nullptr, flip);
}

int TextureManager::DetectBottomOpaqueRow(const char* fileName) {
    // Similar fallback for DetectBottomOpaqueRow: try IMG_Load first, then
    // SDL_IOFromFile/IMG_LoadIO for packaged assets on Android.
    SDL_Surface* surf = IMG_Load(fileName);
    if (!surf) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load failed for '%s' in DetectBottomOpaqueRow: %s", fileName ? fileName : "(null)", SDL_GetError());
        SDL_IOStream* io = SDL_IOFromFile(fileName, "rb");
        if (io) {
            surf = IMG_Load_IO(io, 1);
            if (!surf) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load_IO failed for '%s' in DetectBottomOpaqueRow: %s", fileName ? fileName : "(null)", SDL_GetError());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_IOFromFile returned NULL for '%s' in DetectBottomOpaqueRow", fileName ? fileName : "(null)");
        }
    }
    if (!surf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DetectBottomOpaqueRow: could not load surface for '%s'", fileName ? fileName : "(null)");
        return -1;
    }

    const SDL_PixelFormatDetails* fmtdet = SDL_GetPixelFormatDetails(surf->format);
    int bpp = fmtdet->bytes_per_pixel;

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
            // SDL_GetRGBA expects an SDL_PixelFormat* (surf->format).
            SDL_GetRGBA(pixel, fmtdet, nullptr, &r, &g, &b, &a);
            if (a >= ALPHA_THRESHOLD) opaqueCount++;
        }

        if (opaqueCount >= (int)(surf->w * MIN_FRACTION)) {
            if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);
            int result = y;
            SDL_DestroySurface(surf);
            return result;
        }
    }

    if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);
    int fallback = surf->h - 1;
    SDL_DestroySurface(surf);
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
        // Try to read via SDL_IOFromFile first (works for packaged APK assets),
        // fall back to fopen for desktop where assets may be real files.
        SDL_IOStream* io = SDL_IOFromFile(path, "r");
        if (io) {
            Sint64 sz = SDL_GetIOSize(io);
            if (sz > 0) {
                std::string buf;
                buf.resize((size_t)sz);
                SDL_ReadIO(io, &buf[0], (size_t)sz);
                SDL_CloseIO(io);
                // Manually split into lines to avoid relying on <sstream> on
                // some Android toolchain configurations.
                size_t start = 0;
                while (start < buf.size()) {
                    size_t pos = buf.find('\n', start);
                    std::string line;
                    if (pos == std::string::npos) {
                        line = buf.substr(start);
                        start = buf.size();
                    } else {
                        line = buf.substr(start, pos - start);
                        start = pos + 1;
                    }
                    // Trim leading whitespace
                    size_t i = 0; while (i < line.size() && isspace((unsigned char)line[i])) i++;
                    if (i >= line.size()) continue;
                    if (line[i] == '#') continue;
                    std::string token = line.substr(i);
                    char img[384]; int val = -1;
                    if (sscanf(token.c_str(), "%383s %d", img, &val) == 2) {
                        overrides[std::string(img)] = val;
                    }
                }
            } else {
                SDL_CloseIO(io);
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