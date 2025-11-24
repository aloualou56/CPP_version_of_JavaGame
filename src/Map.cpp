#include "Map.hpp"
#include "TextureManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Map::Map() {
    // Load textures for different tile types
    grass = TextureManager::LoadTexture("sprites/tilesets/16x16 set/grass1.png");
    dirt = TextureManager::LoadTexture("sprites/tilesets/16x16 set/dirt1.png");
    water = TextureManager::LoadTexture("sprites/tilesets/16x16 set/grass2.png");
    stone = TextureManager::LoadTexture("sprites/tilesets/16x16 set/dirt2.png");

    // Initialize map array
    for(int i = 0; i < MAP_HEIGHT; i++) {
        for(int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = 0;
        }
    }

    // Try to load map from file
    loadMapFromFile("maps/map01.txt");

    src.x = src.y = 0;
    src.w = src.h = 16;  // Source tile is 16x16
    
    dest.w = dest.h = TILE_SIZE;  // Destination is scaled to 96x96
}

Map::~Map() {
    SDL_DestroyTexture(grass);
    SDL_DestroyTexture(dirt);
    SDL_DestroyTexture(water);
    SDL_DestroyTexture(stone);
}

void Map::loadMapFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Warning: Could not load map file: " << filepath << std::endl;
        std::cout << "Using default empty map" << std::endl;
        return;
    }

    std::string line;
    int row = 0;
    
    while (std::getline(file, line) && row < MAP_HEIGHT) {
        std::istringstream iss(line);
        int col = 0;
        int value;
        
        while (iss >> value && col < MAP_WIDTH) {
            map[row][col] = value;
            col++;
        }
        row++;
    }
    
    file.close();
    std::cout << "Map loaded successfully from " << filepath << std::endl;
}

void Map::DrawMap(Camera* camera) {
    int type = 0;
    
    // Calculate which tiles are visible
    int startCol = camera->getX() / TILE_SIZE;
    int endCol = (camera->getX() + camera->getWidth()) / TILE_SIZE + 1;
    int startRow = camera->getY() / TILE_SIZE;
    int endRow = (camera->getY() + camera->getHeight()) / TILE_SIZE + 1;
    
    // Clamp to map bounds
    if (startCol < 0) startCol = 0;
    if (endCol > MAP_WIDTH) endCol = MAP_WIDTH;
    if (startRow < 0) startRow = 0;
    if (endRow > MAP_HEIGHT) endRow = MAP_HEIGHT;
    
    // Only draw visible tiles
    for(int row = startRow; row < endRow; row++) {
        for(int column = startCol; column < endCol; column++) {
            type = map[row][column];

            // Calculate world position
            int worldX = column * TILE_SIZE;
            int worldY = row * TILE_SIZE;
            
            // Convert to screen position
            dest.x = camera->worldToScreenX(worldX);
            dest.y = camera->worldToScreenY(worldY);

            switch (type) {
            case 0:
                TextureManager::Draw(grass, src, dest);
                break;
            case 1:
                TextureManager::Draw(dirt, src, dest);
                break;
            case 2:
                TextureManager::Draw(water, src, dest);
                break;
            case 3:
                TextureManager::Draw(stone, src, dest);
                break;
            default:
                TextureManager::Draw(grass, src, dest);
                break;
            }
        }
    }
}

bool Map::isSolidTile(int tileType) const {
    // Tiles that block movement
    return (tileType == 1 || tileType == 3);
}

int Map::getTileAt(int x, int y) const {
    int col = x / TILE_SIZE;
    int row = y / TILE_SIZE;
    
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) {
        return -1;  // Out of bounds
    }
    
    return map[row][col];
}