#ifndef Collision_hpp
#define Collision_hpp
<<<<<<< HEAD
#include <SDL.h>
=======
#include <SDL3/SDL.h>
>>>>>>> SDL3

class Map;

class Collision {
    public:
<<<<<<< HEAD
      static bool AABB(const SDL_Rect& recA, const SDL_Rect& recB);
=======
      static bool AABB(const SDL_FRect& recA, const SDL_FRect& recB);
>>>>>>> SDL3
      static bool checkTileCollision(float x, float y, Map* map);
};



#endif
