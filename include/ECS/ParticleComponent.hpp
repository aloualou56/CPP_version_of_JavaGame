#ifndef ParticleComponent_hpp
#define ParticleComponent_hpp

#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "../Vector2D.hpp"
#include <SDL3/SDL.h>

class ParticleComponent : public Component {
public:
    ParticleComponent() = default;
    ParticleComponent(float lifeMs, float vx, float vy, float damping = 0.95f);
    ~ParticleComponent() override = default;

    void init() override;
    void update() override;
    bool isDrawable() override { return false; }

private:
    PositionComponent* position = nullptr;
    Vector2D velocity{0,0};
    float damping = 0.95f;
    float lifetime = 500.0f; // ms
    Uint32 born = 0;
};

#endif
