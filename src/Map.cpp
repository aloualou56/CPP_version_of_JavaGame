#include <Map.hpp>
#include <TextureManager.hpp>
#include <sstream>
#include <iostream>

Map::Map() {
    loadTileTypes();

    // Αρχικοποιεί τον πίνακα του χάρτη
    for(int i = 0; i < MAP_HEIGHT; i++) {
        for(int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = 0;
        }
    }

    // Προσπαθεί να φορτώσει χάρτη από αρχείο
    loadMapFromFile("maps/map01.txt");

    src.x = src.y = 0.0f;
    src.w = src.h = 16.0f;

    dest.w = dest.h = (float)TILE_SIZE;  // Προορισμός κλιμακωμένος σε 96x96
}

Map::~Map() {
    // Textures are cached and owned by TextureManager; do not destroy here.
}

// Loads the same 35-slot tile catalog as the Java TileManager (same tile
// IDs, same collision flags per ID) so any Java map file behaves correctly
// here, not just the ids this particular map01.txt happens to use. Where the
// original Java-only art (texture_sprites/*) isn't present in this repo, we
// fall back to a visually reasonable asset that already exists here; the
// collision flag always matches Java exactly regardless of the fallback art.
void Map::loadTileTypes() {
    auto set = [this](int id, const char* path, bool collision) {
        SDL_Texture* tex = TextureManager::LoadTexture(path);
        tileTypes[id].texture = tex;
        tileTypes[id].collision = collision;
        float w = 16.0f, h = 16.0f;
        if (tex) SDL_GetTextureSize(tex, &w, &h);
        tileTypes[id].srcW = w;
        tileTypes[id].srcH = h;
    };

    set(0, "sprites/tilesets/16x16 set/grass1.png", false);
    set(1, "sprites/tilesets/16x16 set/grass2.png", false);
    set(2, "sprites/tilesets/16x16 set/grass3.png", false);
    set(3, "sprites/tilesets/grass.png", false);
    set(4, "sprites/tilesets/16x16 set/grass4.png", false);
    set(5, "sprites/tilesets/16x16 set/dirt1.png", false);
    set(6, "sprites/tilesets/16x16 set/dirt2.png", false);
    set(7, "sprites/tilesets/16x16 set/dirt3.png", false);
    set(8, "sprites/tilesets/16x16 set/dirt4.png", false);
    // 9-15: Java's road03-09 (solid paths). No matching art in this repo yet;
    // reuse dirt1 for now, but keep the same collision Java uses.
    for (int id = 9; id <= 15; ++id) set(id, "sprites/tilesets/16x16 set/dirt1.png", true);
    // 16-18: Java's road10-12 (non-solid decorative road)
    for (int id = 16; id <= 18; ++id) set(id, "sprites/tilesets/16x16 set/dirt2.png", false);
    set(19, "sprites/objects/table.png", false);
    set(20, "sprites/objects/tree.png", false); // matches Java: the tile itself isn't flagged solid
    set(21, "sprites/tilesets/walls/walls.png", true); // border wall
    // 22-34: Java's water00-13 (solid)
    for (int id = 22; id <= 34; ++id) set(id, "sprites/tilesets/water_decorations.png", true);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Map tile catalog loaded (%d types)", NUM_TILE_TYPES);
}

void Map::loadMapFromFile(const std::string& filepath) {
    // Read through SDL's I/O instead of std::ifstream: on Android the map is
    // packed inside the APK, which only SDL (via the asset manager) can open.
    size_t size = 0;
    void* data = SDL_LoadFile(filepath.c_str(), &size);
    if (!data) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not load map file %s (%s) - using default empty map", filepath.c_str(), SDL_GetError());
        return;
    }
    std::istringstream file(std::string(static_cast<const char*>(data), size));
    SDL_free(data);

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Map loaded successfully from %s", filepath.c_str());
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
            if (type < 0 || type >= NUM_TILE_TYPES || tileTypes[type].texture == nullptr) {
                type = 0; // unknown/missing tile id falls back to grass, like Java's implicit default
            }

            // Υπολογίζει θέση στον κόσμο
            int worldX = column * TILE_SIZE;
            int worldY = row * TILE_SIZE;

            // Μετατρέπει σε θέση οθόνης
            dest.x = (float)camera->worldToScreenX(worldX);
            dest.y = (float)camera->worldToScreenY(worldY);

            if (type != GROUND_TILE) {
                SDL_FRect groundSrc{ 0.0f, 0.0f, tileTypes[GROUND_TILE].srcW, tileTypes[GROUND_TILE].srcH };
                TextureManager::Draw(tileTypes[GROUND_TILE].texture, groundSrc, dest);
            }

            src.w = tileTypes[type].srcW;
            src.h = tileTypes[type].srcH;
            TextureManager::Draw(tileTypes[type].texture, src, dest);
        }
    }
}

bool Map::isSolidTile(int tileType) const {
    if (tileType < 0 || tileType >= NUM_TILE_TYPES) return false;
    return tileTypes[tileType].collision;
}

int Map::getTileAt(int x, int y) const {
    int col = x / TILE_SIZE;
    int row = y / TILE_SIZE;

    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) {
        return -1;  // Εξωτερικό των ορίων
    }

    return map[row][col];
}

int Map::getTileIdAtCell(int col, int row) const {
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) {
        return -1;
    }
    return map[row][col];
}
