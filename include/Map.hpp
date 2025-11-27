#ifndef Map_hpp
#define Map_hpp
#include "Game.hpp"
#include "Camera.hpp"
#include <string>

class Map {
    public:
       
       Map();
       ~Map();

       void loadMapFromFile(const std::string& filepath);
       void DrawMap(Camera* camera);
       
       bool isSolidTile(int tileType) const;
       int getTileAt(int x, int y) const;

    private:
<<<<<<< HEAD
       SDL_Rect src, dest;
=======
       SDL_FRect src, dest;
>>>>>>> SDL3
       SDL_Texture* dirt;
       SDL_Texture* grass;
       SDL_Texture* water;
       SDL_Texture* stone;
       
       static const int MAP_WIDTH = 50;
       static const int MAP_HEIGHT = 50;
      static const int TILE_SIZE = 96;  // 16 * 6 κλίμακα

       int map[MAP_HEIGHT][MAP_WIDTH];
};



#endif
