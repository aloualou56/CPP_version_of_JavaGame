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
        // Use continuous keyboard state checking instead of discrete events
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        
        // Reset velocity every frame
        position->velocity.x = 0;
        position->velocity.y = 0;
        
        // Track if any movement is happening
        bool isMoving = false;
        
        // Check all movement keys and set velocity
        if (keyState[SDL_SCANCODE_W]) {
            position->velocity.y = -1;
            isMoving = true;
        }
        if (keyState[SDL_SCANCODE_S]) {
            position->velocity.y = 1;
            isMoving = true;
        }
        if (keyState[SDL_SCANCODE_A]) {
            position->velocity.x = -1;
            isMoving = true;
        }
        if (keyState[SDL_SCANCODE_D]) {
            position->velocity.x = 1;
            isMoving = true;
        }
        
        // Update animation based on movement state
        if (animation) {
            if (isMoving) {
                animation->play("Walk");
            } else {
                animation->play("Idle");
            }
        }
      }
};

#endif
