#ifndef SpriteComponent_hpp
#define SpriteComponent_hpp

#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "SDL.h"
#include <iostream>

// Προκαταρκτικές δηλώσεις (forward declarations) για μη-Component κλάσεις
class TextureManager;
class Game;
class Camera;

class SpriteComponent : public Component {
    private:
      PositionComponent *position;
      SDL_Texture *texture;
      SDL_Rect srcRect, destRect;
  std::string texturePath;

    public:
      
      SpriteComponent() = default;
      SpriteComponent(const char* path);
      ~SpriteComponent();

      void setTex(const char* path);
      const std::string& getPath() const { return texturePath; }
      void init() override;
      void update() override;
      void draw() override;
      bool isDrawable() override { return true; }
      // Χρησιμοποιεί το κάτω μέρος του προορισμού (destRect) ώστε οι οντότητες να ταξινομούνται με βάση τα πόδια
      // (αποτρέπει τους χαρακτήρες να σχεδιάζονται πίσω από αντικείμενα που επικαλύπτουν το κάτω τμήμα τους)
      int drawOrder() override { return destRect.y + destRect.h; }
};

#endif