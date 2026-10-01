#ifndef CombatComponent_hpp
#define CombatComponent_hpp

#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
#include <string>
#include <algorithm>
#include "SDL3/SDL.h"

// Shared combat/HP state for anything that can be hit: the player, enemies
// and NPCs. Mirrors the Java Entity base class's takeDamage()/drawHpBar()
// plus Player's death/respawn handling, ported onto this ECS as one
// component instead of base-class fields.
class CombatComponent : public Component {
public:
    float maxHp = 1.0f;
    float currentHp = 1.0f;
    bool isDead = false;
    std::string tag; // "player" / "enemy" / "npc"

    CombatComponent() = default;
    CombatComponent(float max, const std::string& t) : maxHp(max), currentHp(max), tag(t) {}

    void init() override;
    void update() override;
    bool isDrawable() override { return true; }
    int drawOrder() override;
    void draw() override;

    bool isInvulnerable() const { return SDL_GetTicks() < invulnerableUntil; }

    // 0 = just died, 1 = fully through the death window. Used to fade out
    // enemy/NPC sprites; the player instead swaps to literal dead-pose
    // sprite frames so it doesn't need this.
    float deathProgress() const {
        if (!isDead || deathDurationMs == 0) return 0.0f;
        Uint32 elapsed = SDL_GetTicks() - deathAt;
        return std::min(1.0f, (float)elapsed / (float)deathDurationMs);
    }

    // Applies damage from an attack/contact originating at (sourceWorldX,
    // sourceWorldY): knockback away from the source plus a brief
    // invulnerability window so one swing can't hit the same target every
    // frame it overlaps.
    void takeDamage(float amount, float sourceWorldX, float sourceWorldY);

    float getCurrent() const { return currentHp; }
    float getMax() const { return maxHp; }

private:
    PositionComponent* position = nullptr;
    Uint32 invulnerableUntil = 0;
    Uint32 deathAt = 0;
    Uint32 deathDurationMs = 900;
    float spawnX = 0.0f;
    float spawnY = 0.0f;

    void respawnPlayer();
};

#endif
