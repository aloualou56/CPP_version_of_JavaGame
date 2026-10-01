#include <Collision.hpp>
#include <Map.hpp>
#include <cmath>
#include <algorithm>

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

namespace {
    // Like Java's Math.floorDiv: rounds toward negative infinity instead of
    // toward zero, so a position a few pixels past the map edge correctly
    // maps to column/row -1 instead of being mistaken for column/row 0.
    int floorDivInt(int a, int b) {
        int q = a / b;
        int r = a % b;
        if (r != 0 && ((r < 0) != (b < 0))) {
            --q;
        }
        return q;
    }
}

bool Collision::isBoxBlocked(const SDL_FRect& box, Map* map) {
    if (map == nullptr) return false;

    int left   = (int)std::floor(box.x);
    int right  = (int)std::floor(box.x + box.w);
    int top    = (int)std::floor(box.y);
    int bottom = (int)std::floor(box.y + box.h);

    int tileSize = map->getTileSize();
    int leftCol   = floorDivInt(left, tileSize);
    int rightCol  = floorDivInt(right, tileSize);
    int topRow    = floorDivInt(top, tileSize);
    int bottomRow = floorDivInt(bottom, tileSize);

    if (leftCol < 0 || rightCol >= map->getWidthTiles() ||
        topRow < 0 || bottomRow >= map->getHeightTiles()) {
        return true; // off the edge of the world: always blocked
    }

    int cornersCol[4] = { leftCol, rightCol, leftCol, rightCol };
    int cornersRow[4] = { topRow, topRow, bottomRow, bottomRow };
    for (int i = 0; i < 4; ++i) {
        int tileId = map->getTileIdAtCell(cornersCol[i], cornersRow[i]);
        if (map->isSolidTile(tileId)) {
            return true;
        }
    }

    return false;
}

void Collision::clampInsideWorld(float& x, float& y, float w, float h, Map* map) {
    if (map == nullptr) return;

    float maxX = (float)(map->getWidthTiles() * map->getTileSize()) - w;
    float maxY = (float)(map->getHeightTiles() * map->getTileSize()) - h;

    x = std::max(0.0f, std::min(maxX, x));
    y = std::max(0.0f, std::min(maxY, y));
}
