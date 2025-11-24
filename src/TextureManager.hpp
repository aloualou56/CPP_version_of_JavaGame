#ifndef TextureManager_hpp
#define TextureManager_hpp

#include "Game.hpp"

class TextureManager {

    public:
      static SDL_Texture* LoadTexture(const char* fileName);
      // Συμβατότητα προς τα πίσω για ζωγραφική (χωρίς flip)
      static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest);
      // Σχεδιάζει με προαιρετικό flip: δώστε `SDL_FLIP_HORIZONTAL` για κατοπτρισμό
      static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip /*= SDL_FLIP_NONE*/);
};



#endif