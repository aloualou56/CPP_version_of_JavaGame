#include <Collision.hpp>
#include <Map.hpp>

bool Collision::AABB(const SDL_FRect& recA, const SDL_FRect& recB) {
    // Τυπική δοκιμή επικάλυψης AABB (χωρίς επιπλέον περιθώρια)
    if (recA.x < recB.x + recB.w &&
        recA.x + recA.w > recB.x &&
        recA.y < recB.y + recB.h &&
        recA.y + recA.h > recB.y) {
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
