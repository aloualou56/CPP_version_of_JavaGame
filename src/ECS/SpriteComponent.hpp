#ifndef SpriteComponent_hpp
#define SpriteComponent_hpp

#include "Componets.hpp"
#include "SDL.h"
#include "../Camera.hpp"


class SpriteComponent : public Component {
    private:
      PositionComponent *position;
      SDL_Texture *texture;
      SDL_Rect srcRect, destRect;

    public:
      
      SpriteComponent() = default;
      SpriteComponent(const char* path) {

        setTex(path); //kalei to void setTex gia na kanei load to texture
        std::cout << "loaded " << path << std::endl;

      }
      ~SpriteComponent() {
        SDL_DestroyTexture(texture);
      }

      void setTex(const char* path) {
        texture = TextureManager::LoadTexture(path);
      }

      void init() override {

        position = &entity->getComponent<PositionComponent>();

        srcRect.x = srcRect.y = 0;
        srcRect.w = position->width;
        srcRect.h = position->height;
        

      }

      void update() override {

        // Convert world position to screen position using camera
        if (Game::camera) {
            destRect.x = Game::camera->worldToScreenX(position->position.x);
            destRect.y = Game::camera->worldToScreenY(position->position.y);
        } else {
            destRect.x = static_cast<int>(position->position.x);
            destRect.y = static_cast<int>(position->position.y);
        }
        destRect.w = position->width * position->scale;
        destRect.h = position->height * position->scale;

      }

      void draw() override {

        TextureManager::Draw(texture, srcRect, destRect);

      }
};

#endif