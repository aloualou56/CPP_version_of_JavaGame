#include <ECS/EnemyAIComponent.hpp>
#include <ECS/CombatComponent.hpp>
#include <Game.hpp>
#include <Collision.hpp>
#include <cmath>
#include <cstdlib>

void EnemyAIComponent::init() {
    position = &entity->getComponent<PositionComponent>();
}

void EnemyAIComponent::wander() {
    Uint32 now = SDL_GetTicks();
    if (now >= wanderChangeAt) {
        wanderChangeAt = now + 1000 + (Uint32)(rand() % 1500);
        int choice = rand() % 5;
        wanderVX = 0.0f;
        wanderVY = 0.0f;
        switch (choice) {
            case 0: wanderVY = -1.0f; break;
            case 1: wanderVY = 1.0f; break;
            case 2: wanderVX = -1.0f; break;
            case 3: wanderVX = 1.0f; break;
            default: break; // stand still
        }
    }
    position->velocity.x = wanderVX;
    position->velocity.y = wanderVY;
}

void EnemyAIComponent::update() {
    bool selfDead = entity->hasComponent<CombatComponent>() && entity->getComponent<CombatComponent>().isDead;
    if (selfDead) {
        position->velocity.Zero();
        return;
    }

    if (!Game::playerEntity || !Game::playerEntity->hasComponent<PositionComponent>()) {
        position->velocity.Zero();
        return;
    }

    auto& playerPos = Game::playerEntity->getComponent<PositionComponent>();
    bool playerDead = Game::playerEntity->hasComponent<CombatComponent>() && Game::playerEntity->getComponent<CombatComponent>().isDead;

    float centerX = position->position.x + (position->width * position->scale) / 2.0f;
    float centerY = position->position.y + (position->height * position->scale) / 2.0f;
    float targetCenterX = playerPos.position.x + (playerPos.width * playerPos.scale) / 2.0f;
    float targetCenterY = playerPos.position.y + (playerPos.height * playerPos.scale) / 2.0f;

    float dx = targetCenterX - centerX;
    float dy = targetCenterY - centerY;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (!playerDead && dist <= detectionRange) {
        // Snap the vector toward the player onto the same up/down/left/right
        // axes the collision/movement system understands (no true diagonals
        // needed here - matches Java's Enemy.resolveDirection intent).
        position->velocity.x = (dx > 8.0f) ? 1.0f : (dx < -8.0f ? -1.0f : 0.0f);
        position->velocity.y = (dy > 8.0f) ? 1.0f : (dy < -8.0f ? -1.0f : 0.0f);
    } else {
        wander();
    }

    if (!playerDead && dist <= attackRange && SDL_GetTicks() >= contactCooldownUntil) {
        if (Game::playerEntity->hasComponent<CombatComponent>()) {
            Game::playerEntity->getComponent<CombatComponent>().takeDamage(contactDamage, position->position.x, position->position.y);
            // Knockback can shove the player through a wall or off the map
            // edge, so it needs the same hard clamp as ordinary movement.
            Collision::clampInsideWorld(playerPos.position.x, playerPos.position.y, (float)(playerPos.width * playerPos.scale), (float)(playerPos.height * playerPos.scale), Game::map);
        }
        contactCooldownUntil = SDL_GetTicks() + 1000;
    }
}
