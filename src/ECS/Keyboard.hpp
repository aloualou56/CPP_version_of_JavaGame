#ifndef Keyboard_hpp
#define Keyboard_hpp

#include "../Game.hpp"
#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "AnimationComponent.hpp"

class Keyboard : public Component {
    public:
      PositionComponent *position;
      AnimationComponent *animation;

      void init() override {
        position = &entity->getComponent<PositionComponent>();
        // Ensure AnimationComponent exists before trying to get it, although order in Game.cpp ensures it
        if (entity->hasComponent<AnimationComponent>()) {
            animation = &entity->getComponent<AnimationComponent>();
        } else {
            animation = nullptr;
        }
      }

      void update() override {
        if(Game::event.type == SDL_KEYDOWN) {
            switch (Game::event.key.keysym.sym) {
                case SDLK_w:
                 position->velocity.y = -1;
                 if (animation) animation->play("Walk");
                 break;

                case SDLK_a:
                 position->velocity.x = -1;
                 if (animation) animation->play("Walk");
                 break;

                case SDLK_d:
                 position->velocity.x = 1;
                 if (animation) animation->play("Walk");
                 break;

                case SDLK_s:
                 position->velocity.y = 1;
                 if (animation) animation->play("Walk");
                 break;
                
                default:
                 break;
            }

        }
        if(Game::event.type == SDL_KEYUP) {
            switch (Game::event.key.keysym.sym) {
                case SDLK_w:
                 position->velocity.y = 0;
                 if (position->velocity.x == 0 && animation) animation->play("Idle");
                 break;

                case SDLK_a:
                 position->velocity.x = 0;
                 if (position->velocity.y == 0 && animation) animation->play("Idle");
                 break;

                case SDLK_d:
                 position->velocity.x = 0;
                 if (position->velocity.y == 0 && animation) animation->play("Idle");
                 break;

                case SDLK_s:
                 position->velocity.y = 0;
                 if (position->velocity.x == 0 && animation) animation->play("Idle");
                 break;
                
                default:
                 break;
            }
        }
      }
};

#endif
