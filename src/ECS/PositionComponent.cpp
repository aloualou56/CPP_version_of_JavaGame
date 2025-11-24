#include "PositionComponent.hpp"
#include "../Game.hpp"
#include "../Collision.hpp"

void PositionComponent::update() {
    // Store old position
    float oldX = position.x;
    float oldY = position.y;
    
    // Apply movement
    position.x += velocity.x * speed;
    position.y += velocity.y * speed;
    
    // Check tile collision and revert if colliding with solid tiles
    if (Game::map && Collision::checkTileCollision(position.x, position.y, Game::map)) {
        position.x = oldX;
        position.y = oldY;
    }
}
