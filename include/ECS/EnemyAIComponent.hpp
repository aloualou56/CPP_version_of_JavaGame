#ifndef EnemyAIComponent_hpp
#define EnemyAIComponent_hpp

#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
#include "SDL3/SDL.h"

// A simple hostile enemy: wanders the map until the player wanders inside
// its detection range, then chases and deals contact damage on touch. Port
// of Java's Entity/Enemy.java onto this ECS - movement/tile collision comes
// for free from the shared PositionComponent::update() since this just sets
// velocity like the player's Keyboard component does.
class EnemyAIComponent : public Component {
public:
    float detectionRange = 384.0f; // 4 tiles, matches Java (tileSize*4, both use 96px tiles)
    float attackRange = 67.0f;     // matches Java (tileSize*0.7)
    float contactDamage = 0.5f;    // rescaled onto the 5-heart player HP pool (~10 hits to kill, like Java's 100/10)

    void init() override;
    void update() override;

private:
    PositionComponent* position = nullptr;
    Uint32 wanderChangeAt = 0;
    float wanderVX = 0.0f;
    float wanderVY = 0.0f;
    Uint32 contactCooldownUntil = 0;

    void wander();
};

#endif
