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

       // Direct grid accessors (col/row, not pixels) used by the tile-based
       // collision check so it can test the exact tiles under a box's
       // corners instead of a single sampled pixel.
       int getTileIdAtCell(int col, int row) const;
       int getTileSize() const { return TILE_SIZE; }
       int getWidthTiles() const { return MAP_WIDTH; }
       int getHeightTiles() const { return MAP_HEIGHT; }

    private:
       SDL_FRect src, dest;

       static const int MAP_WIDTH = 50;
       static const int MAP_HEIGHT = 50;
       static const int TILE_SIZE = 96;  // 16 * 6 κλίμακα
       static const int NUM_TILE_TYPES = 35;

       struct TileType {
           SDL_Texture* texture = nullptr;
           bool collision = false;
           float srcW = 16.0f;
           float srcH = 16.0f;
       };
       TileType tileTypes[NUM_TILE_TYPES];
       void loadTileTypes();

       int map[MAP_HEIGHT][MAP_WIDTH];
};



#endif
