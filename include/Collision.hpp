#ifndef Collision_hpp
#define Collision_hpp
#include <SDL3/SDL.h>

class Map;

class Collision {
    public:
      static bool AABB(const SDL_FRect& recA, const SDL_FRect& recB);
      static bool checkTileCollision(float x, float y, Map* map);

      // Checks the fully combined destination box (both axes moved at once,
      // i.e. a diagonal move is validated as one shape) against every tile
      // its four corners land on, plus an explicit map-edge bounds check.
      // This replaces validating each axis independently, which is what let
      // a diagonal move slip past the map border in the original bug: two
      // per-axis checks against the *current* (pre-move) box can each look
      // clear on their own even when the actual combined destination has
      // walked off the edge of the world.
      static bool isBoxBlocked(const SDL_FRect& box, Map* map);

      // Last-resort safety net: forces a box to stay within the map's pixel
      // bounds no matter what got it out of position (bad input timing, a
      // future bug, a large frame hitch, etc.), so the game can never end up
      // indexing tiles with an out-of-range world position.
      static void clampInsideWorld(float& x, float& y, float w, float h, Map* map);
};



#endif
