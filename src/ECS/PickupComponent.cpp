#include <ECS/PickupComponent.hpp>
#include <ECS/InventoryComponent.hpp>
#include <Collision.hpp>
#include <Game.hpp>

void PickupComponent::init() {
    position = &entity->getComponent<PositionComponent>();
}

void PickupComponent::update() {
    if (!Game::playerEntity) return;
    if (!Game::playerEntity->hasComponent<PositionComponent>() || !Game::playerEntity->hasComponent<InventoryComponent>()) return;

    auto& pp = Game::playerEntity->getComponent<PositionComponent>();
    SDL_FRect playerBox{ pp.position.x, pp.position.y, (float)(pp.width * pp.scale), (float)(pp.height * pp.scale) };
    SDL_FRect myBox{
        position->position.x - margin,
        position->position.y - margin,
        (float)(position->width * position->scale) + margin * 2.0f,
        (float)(position->height * position->scale) + margin * 2.0f
    };
    if (!Collision::AABB(playerBox, myBox)) return;

    auto& inv = Game::playerEntity->getComponent<InventoryComponent>();
    if (type == Type::Key) {
        inv.keys++;
        entity->destroy();
    } else if (type == Type::Door) {
        if (inv.keys > 0) {
            inv.keys--;
            entity->destroy();
        }
    }
}
