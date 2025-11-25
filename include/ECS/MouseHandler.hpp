#ifndef MouseHandler_hpp
#define MouseHandler_hpp

#include <Game.hpp>
#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>

class MouseHandler : public Component {
public:
    PositionComponent* position;
    bool attacking = false;
    
    void init() override {
        position = &entity->getComponent<PositionComponent>();
    }
    
    void update() override {
        attacking = false;
        
        // Ελέγχει για πάτημα κουμπιού ποντικιού
        if (Game::event.type == SDL_MOUSEBUTTONDOWN) {
            if (Game::event.button.button == SDL_BUTTON_LEFT) {
                attacking = true;
            }
        }
        
        // Επίσης ελέγχει για επίθεση με πλήκτρο R (εναλλακτική από πληκτρολόγιο)
        if (Game::event.type == SDL_KEYDOWN) {
            if (Game::event.key.keysym.sym == SDLK_r) {
                attacking = true;
            }
        }
    }
};

#endif
