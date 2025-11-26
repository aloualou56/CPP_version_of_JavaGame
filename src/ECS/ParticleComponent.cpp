#include <ECS/ParticleComponent.hpp>
#include <ECS/ECS.hpp>
#include <SDL3/SDL.h>

ParticleComponent::ParticleComponent(float lifeMs, float vx, float vy, float damp) {
    lifetime = lifeMs;
    velocity.x = vx;
    velocity.y = vy;
    damping = damp;
}

void ParticleComponent::init() {
    position = &entity->getComponent<PositionComponent>();
    born = SDL_GetTicks();
}

void ParticleComponent::update() {
    if (!position) return;
    // Move particle by its velocity
    position->position.x += velocity.x;
    position->position.y += velocity.y;
    // Apply damping
    velocity.x *= damping;
    velocity.y *= damping;
    // Lifetime check
    Uint32 now = SDL_GetTicks();
    if (now - born >= (Uint32)lifetime) {
        entity->destroy();
    }
}
