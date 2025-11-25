#ifndef TextureManager_hpp
#define TextureManager_hpp

#include "Game.hpp"

class TextureManager {

    public:
      static SDL_Texture* LoadTexture(const char* fileName);
  static void SetTotalToLoad(int total);
  static int GetLoadedCount();
  static int GetTotalToLoad();
  // Scan image and return the lowest non-transparent row (pixel Y from top), or -1 on error
  static int DetectBottomOpaqueRow(const char* fileName);
    // Return an override anchor row (in pixels from top) for given file, or -1 if none
    static int GetAnchorOverride(const char* fileName);
      // Συμβατότητα προς τα πίσω για ζωγραφική (χωρίς flip)
      static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest);
      // Σχεδιάζει με προαιρετικό flip: δώστε `SDL_FLIP_HORIZONTAL` για κατοπτρισμό
      static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip /*= SDL_FLIP_NONE*/);
};



#endif