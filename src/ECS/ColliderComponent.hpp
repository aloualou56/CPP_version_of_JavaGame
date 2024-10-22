#ifndef ColliderComponent_hpp
#define ColliderComponent_hpp

#include <string>
#include "SDL.h"
#include "Componets.hpp"

class ColliderComponent : public Component {
    public:
     SDL_Rect collider;
     std::string tag;

     PositionComponent *position;

     ColliderComponent(std::string t) {
      tag = t;
     }

     void init() override {
        if(!entity->hasComponent<PositionComponent>()) {
            entity->addComponent<PositionComponent>();
        }
        position = &entity->getComponent<PositionComponent>();
     }

     void update() override {
        collider.x = static_cast<int>(position->position.x);
        collider.y = static_cast<int>(position->position.y);
        collider.w = ((position->width * position->scale) / 2) + 20;
        collider.h = ((position->height * position->scale) / 2); //kati paei lados poli ladso
     }
};

#endif