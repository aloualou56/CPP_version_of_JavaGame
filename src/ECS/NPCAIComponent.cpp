#include <ECS/NPCAIComponent.hpp>
#include <ECS/CombatComponent.hpp>
#include <cstdlib>

void NPCAIComponent::init() {
    position = &entity->getComponent<PositionComponent>();
    animation = entity->hasComponent<AnimationComponent>() ? &entity->getComponent<AnimationComponent>() : nullptr;
}

void NPCAIComponent::update() {
    bool selfDead = entity->hasComponent<CombatComponent>() && entity->getComponent<CombatComponent>().isDead;
    if (selfDead) {
        position->velocity.Zero();
        return;
    }

    Uint32 now = SDL_GetTicks();
    if (now >= wanderChangeAt) {
        wanderChangeAt = now + 1500 + (Uint32)(rand() % 2500);
        int choice = rand() % 5;
        position->velocity.x = 0.0f;
        position->velocity.y = 0.0f;
        switch (choice) {
            case 0: position->velocity.y = -1.0f; break;
            case 1: position->velocity.y = 1.0f; break;
            case 2: position->velocity.x = -1.0f; break;
            case 3: position->velocity.x = 1.0f; break;
            default: break; // stand still
        }
    }

    bool moving = position->velocity.x != 0.0f || position->velocity.y != 0.0f;
    if (animation) {
        if (moving) {
            if (animation->getCurrentAnimation() != "Walk") animation->play("Walk");
            animation->setFlip(position->velocity.x < 0.0f);
        } else {
            if (animation->getCurrentAnimation() != "Idle") animation->play("Idle");
        }
    }
}
