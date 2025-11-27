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
<<<<<<< HEAD
      static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest);
      // Σχεδιάζει με προαιρετικό flip: δώστε `SDL_FLIP_HORIZONTAL` για κατοπτρισμό
      static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip /*= SDL_FLIP_NONE*/);
=======
      static void Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest);
      // Σχεδιάζει με προαιρετικό flip: δώστε `SDL_FLIP_HORIZONTAL` για κατοπτρισμό
      static void Draw(SDL_Texture *tex, SDL_FRect src, SDL_FRect dest, SDL_FlipMode flip /*= SDL_FLIP_NONE*/);
>>>>>>> SDL3
};



#endif
