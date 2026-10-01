#include <ECS/CombatComponent.hpp>
#include <ECS/AnimationComponent.hpp>
#include <Game.hpp>
#include <Camera.hpp>
#include <cmath>
#include <algorithm>

void CombatComponent::init() {
    position = &entity->getComponent<PositionComponent>();
    spawnX = position->position.x;
    spawnY = position->position.y;
}

void CombatComponent::takeDamage(float amount, float sourceWorldX, float sourceWorldY) {
    if (isDead || isInvulnerable()) return;

    currentHp -= amount;
    invulnerableUntil = SDL_GetTicks() + 500;

    float dx = position->position.x - sourceWorldX;
    float dy = position->position.y - sourceWorldY;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0f) { dx = 1.0f; dy = 0.0f; len = 1.0f; }
    const float knockback = 24.0f;
    position->position.x += (dx / len) * knockback;
    position->position.y += (dy / len) * knockback;

    if (currentHp <= 0.0f) {
        currentHp = 0.0f;
        isDead = true;
        deathAt = SDL_GetTicks();
        if (tag == "player" && entity->hasComponent<AnimationComponent>()) {
            entity->getComponent<AnimationComponent>().play("Dead", false);
        }
    }
}

void CombatComponent::respawnPlayer() {
    position->position.x = spawnX;
    position->position.y = spawnY;
    position->velocity.Zero();
    currentHp = maxHp;
    isDead = false;
    invulnerableUntil = SDL_GetTicks() + 1500;
    if (entity->hasComponent<AnimationComponent>()) {
        entity->getComponent<AnimationComponent>().play("Idle", true);
    }
}

void CombatComponent::update() {
    if (!isDead) return;

    if (SDL_GetTicks() - deathAt > deathDurationMs) {
        if (tag == "player") {
            respawnPlayer();
        } else {
            entity->destroy();
        }
    }
}

int CombatComponent::drawOrder() {
    return position ? (int)(position->position.y) : 0;
}

void CombatComponent::draw() {
    if (tag == "player") return; // the player's HP is shown via the heart HUD instead
    if (isDead || currentHp >= maxHp || !Game::renderer) return;

    int screenX = Game::camera ? Game::camera->worldToScreenX(position->position.x) : (int)position->position.x;
    int screenY = Game::camera ? Game::camera->worldToScreenY(position->position.y) : (int)position->position.y;

    int barWidth = position->width * position->scale;
    int barHeight = 6;
    int barY = screenY - 10;
    float ratio = std::max(0.0f, currentHp / maxHp);

    SDL_FRect back{ (float)(screenX - 1), (float)(barY - 1), (float)(barWidth + 2), (float)(barHeight + 2) };
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(Game::renderer, &back);

    SDL_FRect red{ (float)screenX, (float)barY, (float)barWidth, (float)barHeight };
    SDL_SetRenderDrawColor(Game::renderer, 200, 30, 30, 255);
    SDL_RenderFillRect(Game::renderer, &red);

    SDL_FRect green{ (float)screenX, (float)barY, (float)(barWidth * ratio), (float)barHeight };
    SDL_SetRenderDrawColor(Game::renderer, 60, 200, 70, 255);
    SDL_RenderFillRect(Game::renderer, &green);
}
