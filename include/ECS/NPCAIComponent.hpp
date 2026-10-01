#ifndef NPCAIComponent_hpp
#define NPCAIComponent_hpp

#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
#include <ECS/AnimationComponent.hpp>
#include "SDL3/SDL.h"

// A friendly, non-hostile character that wanders the map for atmosphere.
// Never chases or damages the player, but - per the original Java design -
// isn't invulnerable either: the player's attacks can still hurt and kill
// it (see CombatComponent, added alongside this on the same entity).
class NPCAIComponent : public Component {
public:
    void init() override;
    void update() override;

private:
    PositionComponent* position = nullptr;
    AnimationComponent* animation = nullptr;
    Uint32 wanderChangeAt = 0;
};

#endif
