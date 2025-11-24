#ifndef Collision_hpp
#define Collision_hpp
#include <SDL.h>

class Map;

class Collision {
    public:
      static bool AABB(const SDL_Rect& recA, const SDL_Rect& recB);
      static bool checkTileCollision(float x, float y, Map* map);
};



#endif