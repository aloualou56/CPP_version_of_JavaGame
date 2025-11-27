#ifndef SpriteComponent_hpp
#define SpriteComponent_hpp

#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
<<<<<<< HEAD
#include <SDL.h>
=======
#include <SDL3/SDL.h>
>>>>>>> SDL3
#include <iostream>

// Προκαταρκτικές δηλώσεις (forward declarations) για μη-Component κλάσεις
class TextureManager;
class Game;
class Camera;

class SpriteComponent : public Component {
    private:
      PositionComponent *position;
      SDL_Texture *texture;
<<<<<<< HEAD
      SDL_Rect srcRect, destRect;
=======
      SDL_FRect srcRect, destRect;
>>>>>>> SDL3
  std::string texturePath;
      // Anchor (pixels from top) used to compute draw order — defaults to sprite bottom
      int anchorY = 0;

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
      int drawOrder() override { return destRect.y + anchorY; }

      // Επιτρέπει να ρυθμιστεί η κάθετη άγκυρα (σε pixels από την κορυφή της εικόνας)
      void setAnchorY(int a) { anchorY = a; }
};

#endif
