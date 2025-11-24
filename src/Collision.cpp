#include "Collision.hpp"
#include "Map.hpp"

bool Collision::AABB(const SDL_Rect& recA, const SDL_Rect& recB) {
    
    if(recA.x + recA.w >= recB.x && recB.x + recB.w >= recA.x && recA.y + recA.h >= (recB.y - 50) && (recB.y - 50) + (recB.h + 60) >= recA.y) {
        return true;
    }

    return false;
}

bool Collision::checkTileCollision(float x, float y, Map* map) {
    if (map == nullptr) {
        return false;
    }
    
    int tileType = map->getTileAt(static_cast<int>(x), static_cast<int>(y));
    return map->isSolidTile(tileType);
}
