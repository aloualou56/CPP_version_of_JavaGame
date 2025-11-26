#ifndef Collision_hpp
#define Collision_hpp
#include <SDL3/SDL.h>

class Map;

class Collision {
    public:
      static bool AABB(const SDL_FRect& recA, const SDL_FRect& recB);
      static bool checkTileCollision(float x, float y, Map* map);
};



#endif
