#include <Map.hpp>
#include <TextureManager.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Map::Map() {
    // Φορτώνει textures για διαφορετικούς τύπους πλακιδίων
    grass = TextureManager::LoadTexture("sprites/tilesets/16x16 set/grass1.png");
    dirt = TextureManager::LoadTexture("sprites/tilesets/16x16 set/dirt1.png");
    water = TextureManager::LoadTexture("sprites/tilesets/16x16 set/grass2.png");
    stone = TextureManager::LoadTexture("sprites/tilesets/16x16 set/dirt2.png");

    // Αρχικοποιεί τον πίνακα του χάρτη
    for(int i = 0; i < MAP_HEIGHT; i++) {
        for(int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = 0;
        }
    }
<<<<<<< HEAD

    // Προσπαθεί να φορτώσει χάρτη από αρχείο
    loadMapFromFile("maps/map01.txt");

    src.x = src.y = 0;
    src.w = src.h = 16;  // Πηγή πλακιδίου 16x16
    
    dest.w = dest.h = TILE_SIZE;  // Προορισμός κλιμακωμένος σε 96x96
=======

    // Προσπαθεί να φορτώσει χάρτη από αρχείο
    loadMapFromFile("maps/map01.txt");

    src.x = src.y = 0.0f;
    src.w = src.h = 16.0f;  // Πηγή πλακιδίου 16x16
    
    dest.w = dest.h = (float)TILE_SIZE;  // Προορισμός κλιμακωμένος σε 96x96
>>>>>>> SDL3
}

Map::~Map() {
    // Textures are cached and owned by TextureManager; do not destroy here.
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
    
    // Υπολογίζει ποια πλακίδια είναι ορατά
    int startCol = camera->getX() / TILE_SIZE;
    int endCol = (camera->getX() + camera->getWidth()) / TILE_SIZE + 1;
    int startRow = camera->getY() / TILE_SIZE;
    int endRow = (camera->getY() + camera->getHeight()) / TILE_SIZE + 1;
    
    // Περιορίζει στα όρια του χάρτη
    if (startCol < 0) startCol = 0;
    if (endCol > MAP_WIDTH) endCol = MAP_WIDTH;
    if (startRow < 0) startRow = 0;
    if (endRow > MAP_HEIGHT) endRow = MAP_HEIGHT;
    
    // Σχεδιάζει μόνο τα ορατά πλακίδια
    for(int row = startRow; row < endRow; row++) {
        for(int column = startCol; column < endCol; column++) {
            type = map[row][column];

            // Υπολογίζει θέση στον κόσμο
            int worldX = column * TILE_SIZE;
            int worldY = row * TILE_SIZE;
            
            // Μετατρέπει σε θέση οθόνης
<<<<<<< HEAD
            dest.x = camera->worldToScreenX(worldX);
            dest.y = camera->worldToScreenY(worldY);
=======
            dest.x = (float)camera->worldToScreenX(worldX);
            dest.y = (float)camera->worldToScreenY(worldY);
>>>>>>> SDL3

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
    // Πλακίδια που εμποδίζουν την κίνηση
    return (tileType == 1 || tileType == 3);
}

int Map::getTileAt(int x, int y) const {
    int col = x / TILE_SIZE;
    int row = y / TILE_SIZE;
    
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) {
        return -1;  // Εξωτερικό των ορίων
    }
    
    return map[row][col];
}