#include <ECS/AttackComponent.hpp>
#include <ECS/CombatComponent.hpp>
#include <Collision.hpp>
#include <Game.hpp>

void AttackComponent::init() {
    position = &entity->getComponent<PositionComponent>();
    animation = entity->hasComponent<AnimationComponent>() ? &entity->getComponent<AnimationComponent>() : nullptr;
}

void AttackComponent::performSwing() {
    const float tileSize = 96.0f; // matches Map::TILE_SIZE / Java's tileSize
    float reach = tileSize / 2.0f;
    float size = tileSize - 16.0f;

    float centerX = position->position.x + (position->width * position->scale) / 2.0f;
    float centerY = position->position.y + (position->height * position->scale) / 2.0f;

    float offsetX = facingX * reach;
    float offsetY = facingY * reach;

    SDL_FRect hitbox{ centerX + offsetX - size / 2.0f, centerY + offsetY - size / 2.0f, size, size };

    if (!Game::managerPtr) return;
    for (const auto& ePtr : Game::managerPtr->getEntities()) {
        Entity* other = ePtr.get();
        if (!other || other == entity) continue;
        if (!other->hasComponent<CombatComponent>() || !other->hasComponent<PositionComponent>()) continue;

        auto& cc = other->getComponent<CombatComponent>();
        if (cc.tag == "player" || cc.isDead) continue;

        auto& op = other->getComponent<PositionComponent>();
        SDL_FRect targetBox{ op.position.x, op.position.y, (float)(op.width * op.scale), (float)(op.height * op.scale) };
        if (Collision::AABB(hitbox, targetBox)) {
            cc.takeDamage(damage, position->position.x, position->position.y);
            // Knockback can shove the target through a wall or off the map
            // edge, so it needs the same hard clamp as ordinary movement.
            Collision::clampInsideWorld(op.position.x, op.position.y, (float)(op.width * op.scale), (float)(op.height * op.scale), Game::map);
        }
    }
}

void AttackComponent::update() {
    if (entity->hasComponent<CombatComponent>() && entity->getComponent<CombatComponent>().isDead) return;

    // Track facing from this frame's movement input (Keyboard/MouseHandler
    // already updated velocity earlier this same update pass).
    if (position->velocity.x != 0.0f || position->velocity.y != 0.0f) {
        facingX = position->velocity.x;
        facingY = position->velocity.y;
    }

    const bool* keyState = SDL_GetKeyboardState(NULL);
    bool keyHeld = keyState[SDL_SCANCODE_R];
    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    bool mouseHeld = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;
    bool held = keyHeld || mouseHeld;

#ifdef __ANDROID__
    held = held || Game::attackActive;
#endif

    if (held && SDL_GetTicks() >= cooldownUntil) {
        if (animation) animation->play("Attack", false);
        performSwing();
        cooldownUntil = SDL_GetTicks() + cooldownMs;
    }
}
