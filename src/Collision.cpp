#include "Collision.hpp"

bool Collision::AABB(const SDL_Rect& recA, const SDL_Rect& recB) {
    
    if(recA.x + recA.w >= recB.x && recB.x + recB.w >= recA.x && recA.y + recA.h >= (recB.y - 50) && (recB.y - 50) + (recB.h + 60) >= recA.y) {
        return true;
    }

    return false;
}