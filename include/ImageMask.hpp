#ifndef ImageMask_hpp
#define ImageMask_hpp

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class ImageMask {
public:
    // Κατώφλι alpha για εντοπισμό στερεών pixels (200 = ~78% αδιαφάνεια)
    // Pixels με alpha > COLLISION_ALPHA_THRESHOLD θεωρούνται στερεά για σύγκρουση
    static constexpr int COLLISION_ALPHA_THRESHOLD = 200;
    
    int width = 0;
    int height = 0;
    // Μετατοπίσεις της μάσκας σε σχέση με την αρχική εικόνα (περικοπή από πάνω-αριστερά)
    int offsetX = 0;
    int offsetY = 0;
    std::vector<std::vector<bool>> mask; // mask[y][x] == true αν είναι αδιαφανές/στέρεο

    // Φορτώνει μάσκα από PNG αρχείο (όπου τα μη διαφανή pixels θεωρούνται στερεά)
    bool loadFromPNG(const std::string& path) {
        SDL_Surface* loaded = IMG_Load(path.c_str());
        if (!loaded) return false;

        // Εξασφαλίζει μορφή 32-bit RGBA για εύκολη πρόσβαση pixel
        SDL_Surface* surface = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(loaded);
        if (!surface) return false;

        int w = surface->w;
        int h = surface->h;

        // Βρίσκει το περίγραμμα των μη-διαφανών pixels
        int minX = w, minY = h, maxX = -1, maxY = -1;
        SDL_LockSurface(surface);
        Uint8* pixels = static_cast<Uint8*>(surface->pixels);
        int pitch = surface->pitch; // bytes per row
        const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surface->format);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                Uint32 px = *reinterpret_cast<Uint32*>(pixels + y * pitch + x * 4);
                Uint8 r, g, b, a;
                SDL_GetRGBA(px, fmt, nullptr, &r, &g, &b, &a);
                if (a > COLLISION_ALPHA_THRESHOLD) {  // Χρησιμοποιεί υψηλότερο κατώφλι για να αγνοεί ημιδιαφανή pixels
                    if (x < minX) minX = x;
                    if (y < minY) minY = y;
                    if (x > maxX) maxX = x;
                    if (y > maxY) maxY = y;
                }
            }
        }

        // Αν είναι πλήρως διαφανής, παραγάγει κενή μάσκα
        if (maxX < 0 || maxY < 0) {
            SDL_UnlockSurface(surface);
            SDL_DestroySurface(surface);
            width = 0;
            height = 0;
            offsetX = offsetY = 0;
            mask.clear();
            return true;
        }

        // Περικόπτει στο περίγραμμα
        offsetX = minX;
        offsetY = minY;
        width = maxX - minX + 1;
        height = maxY - minY + 1;
        mask.assign(height, std::vector<bool>(width, false));

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Uint32 px = *reinterpret_cast<Uint32*>(pixels + (y + minY) * pitch + (x + minX) * 4);
                Uint8 r, g, b, a;
                SDL_GetRGBA(px, fmt, nullptr, &r, &g, &b, &a);
                mask[y][x] = (a > COLLISION_ALPHA_THRESHOLD);  // Χρησιμοποιεί το ίδιο κατώφλι με την εύρεση περιγράμματος
            }
        }

        SDL_UnlockSurface(surface);
        SDL_DestroySurface(surface);
        return true;
    }

    // Ελέγχει αν το pixel στο (x, y) είναι στερεό
    bool isSolid(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) return false;
        return mask[y][x];
    }
};

#endif
