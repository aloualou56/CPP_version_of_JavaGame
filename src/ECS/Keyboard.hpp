#ifndef Keyboard_hpp
#define Keyboard_hpp

#include "../Game.hpp"
#include "ECS.hpp"
#include "Componets.hpp"

class Keyboard : public Component {
    public:
      PositionComponent *position;

      void init() override {
        position = &entity->getComponent<PositionComponent>();
      }

      void update() override {
        if(Game::event.type == SDL_KEYDOWN) {
            switch (Game::event.key.keysym.sym) {
                case SDLK_w:
                 position->velocity.y = -1;
                 break;

                case SDLK_a:
                 position->velocity.x = -1;
                 break;

                case SDLK_d:
                 position->velocity.x = 1;
                 break;

                case SDLK_s:
                 position->velocity.y = 1;
                 break;
                
                default:
                 break;
            }

        }
        if(Game::event.type == SDL_KEYUP) {
            switch (Game::event.key.keysym.sym) {
                case SDLK_w:
                 position->velocity.y = 0;
                 break;

                case SDLK_a:
                 position->velocity.x = 0;
                 break;

                case SDLK_d:
                 position->velocity.x = 0;
                 break;

                case SDLK_s:
                 position->velocity.y = 0 ;
                 break;
                
                default:
                 break;
            }
        }
      }

      
};

#endif