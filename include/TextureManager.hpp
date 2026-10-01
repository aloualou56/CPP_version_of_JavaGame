#ifndef TextureManager_hpp
#define TextureManager_hpp

#include "Game.hpp"

#include <string>
#include <map>
#include <mutex>
class TextureManager {

    public:
      static SDL_Texture* LoadTexture(const char* fileName);
  static void SetTotalToLoad(int total);
  static int GetLoadedCount();
  static int GetTotalToLoad();
  // Register an externally-created texture under a key so the manager
  // owns and will free it on ClearCache/exit. Useful for procedurally
  // generated textures created at runtime.
  static void RegisterTexture(const char* key, SDL_Texture* tex);
  // Retrieve a registered/cached texture (or nullptr)
  static SDL_Texture* GetTexture(const char* key);
  // Destroy and clear all cached textures
  static void ClearCache();
  // Scan image and return the lowest non-transparent row (pixel Y from top), or -1 on error
  static int DetectBottomOpaqueRow(const char* fileName);
    // Return an override anchor row (in pixels from top) for given file, or -1 if none
    static int GetAnchorOverride(const char* fileName);
      // Συμβατότητα προς τα πίσω για ζωγραφική (χωρίς flip)
      static void Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest);
      // Σχεδιάζει με προαιρετικό flip: δώστε `SDL_FLIP_HORIZONTAL` για κατοπτρισμό
      static void Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest, SDL_FlipMode flip /*= SDL_FLIP_NONE*/);
      // Σχεδιάζει με flip και προσωρινό alpha-mod (0-255). Το alpha
      // επαναφέρεται αμέσως μετά τη σχεδίαση ώστε να μην "μολύνει" άλλες
      // οντότητες που μοιράζονται την ίδια cached υφή.
      static void Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest, SDL_FlipMode flip, Uint8 alpha);

      // Crops a rectangular region out of an already-loaded image file into
      // its own cached texture, registered under `key` (so repeated calls
      // with the same key just hit the cache). Used to slice a single
      // spritesheet file (e.g. a 2-frame slime sheet) into per-frame
      // textures without needing separate image files on disk.
      static SDL_Texture* LoadTextureRegion(const char* sourcePath, const char* key, int x, int y, int w, int h);

      // Loads an image and blends a flat color over its already-opaque
      // pixels (alpha in 0..1), preserving transparency - mirrors the Java
      // NPC's SRC_ATOP tint. Registers the result under `key`.
      static SDL_Texture* LoadTintedTexture(const char* sourcePath, const char* key, Uint8 r, Uint8 g, Uint8 b, float alpha);
};



#endif
