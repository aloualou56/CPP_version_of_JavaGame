#ifndef SpriteComponent_hpp
#define SpriteComponent_hpp

#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "SDL.h"
#include <iostream>

// Forward declarations for non-Component classes
class TextureManager;
class Game;
class Camera;

class SpriteComponent : public Component {
    private:
      PositionComponent *position;
      SDL_Texture *texture;
      SDL_Rect srcRect, destRect;

    public:
      
      SpriteComponent() = default;
      SpriteComponent(const char* path);
      ~SpriteComponent();

      void setTex(const char* path);
      void init() override;
      void update() override;
      void draw() override;
};

#endif