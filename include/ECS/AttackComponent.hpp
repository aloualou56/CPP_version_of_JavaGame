#ifndef AttackComponent_hpp
#define AttackComponent_hpp

#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
#include <ECS/AnimationComponent.hpp>
#include "SDL3/SDL.h"

// Player-only: turns held R/left-click input into an actual melee swing -
// builds a hitbox in front of the player and damages whatever overlaps it.
// Port of Java's Player attack handling + CollisionChecker.checkAttackHit.
class AttackComponent : public Component {
public:
    float damage = 1.0f; // rescaled to match enemy/NPC HP pools (see EnemyAIComponent)
    Uint32 cooldownMs = 350;

    void init() override;
    void update() override;

private:
    PositionComponent* position = nullptr;
    AnimationComponent* animation = nullptr;
    float facingX = 0.0f;
    float facingY = 1.0f; // default facing "down", matches Java's Player default
    Uint32 cooldownUntil = 0;

    void performSwing();
};

#endif
