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
        // Εξασφαλίζει ότι το AnimationComponent υπάρχει πριν το αποκτήσει, αν και η σειρά στο Game.cpp το
        // διασφαλίζει
        if (entity->hasComponent<AnimationComponent>()) {
            animation = &entity->getComponent<AnimationComponent>();
        } else {
            animation = nullptr;
        }
      }

      void update() override {
        // Χρησιμοποιεί συνεχή έλεγχο κατάστασης πληκτρολογίου αντί για διακριτά γεγονότα
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        
        // Επαναφέρει την ταχύτητα κάθε καρέ
        position->velocity.x = 0;
        position->velocity.y = 0;
        
        // Παρακολουθεί αν υπάρχει οποιαδήποτε κίνηση
        bool isMoving = false;
        
        // Ελέγχει όλα τα πλήκτρα κίνησης και θέτει την ταχύτητα
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
        
        // Ενημερώνει το animation με βάση την κατάσταση κίνησης, αλλά δεν διακόπτει μη-επαναλαμβανόμενες animations
        if (animation) {
            if (!animation->isBusy()) {
                if (isMoving) {
                    if (animation->getCurrentAnimation() != "Walk") animation->play("Walk");
                } else {
                    if (animation->getCurrentAnimation() != "Idle") animation->play("Idle");
                }
            }
        }
        // Ορίζει την κατεύθυνση που κοιτάει: A = αριστερά (flip), D = δεξιά (χωρίς flip)
        if (animation) {
            if (keyState[SDL_SCANCODE_A]) {
                animation->setFlip(true);
            } else if (keyState[SDL_SCANCODE_D]) {
                animation->setFlip(false);
            }
            // Attack is handled in Game event loop to ensure a single trigger per keydown
        }
      }
};

#endif
