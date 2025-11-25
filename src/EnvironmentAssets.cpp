#include <EnvironmentAssets.hpp>
#include <iostream>

EnvironmentAssets::EnvironmentAssets(Manager* mgr, int worldW, int worldH, int tileS) 
    : manager(mgr), worldWidth(worldW), worldHeight(worldH), tileSize(tileS) {
    // Σπείρει (seed) τον γεννήτορα τυχαίων αριθμών
    rng.seed(static_cast<unsigned int>(time(nullptr)));
}

EnvironmentAssets::~EnvironmentAssets() {
}

void EnvironmentAssets::generateEnvironment() {
    std::cout << "Generating environment..." << std::endl;
    
    placeGrassDecorations();
    placeBushes();
    placeRocks();
    
    std::cout << "Environment generation complete." << std::endl;
}

void EnvironmentAssets::placeGrassDecorations() {
    // Τοποθετεί τυχαίες διακοσμήσεις χόρτου στον χάρτη
    // Περίπου 100 διακοσμήσεις χόρτου τυχαία κατανεμημένες
    for (int i = 0; i < 100; i++) {
        float x = getRandomX();
        float y = getRandomY();
        int grassType = getRandomGrassType();
        
        EnvironmentFactory::createEnvGrass(*manager, grassType, x, y);
    }
}

void EnvironmentAssets::placeBushes() {
    // Τοποθετεί τυχαίους θάμνους (thamnos tonia)
    // Περίπου 30 θάμνοι
    for (int i = 0; i < 30; i++) {
        float x = getRandomX();
        float y = getRandomY();
        
        EnvironmentFactory::createThamnosTonia(*manager, x, y);
    }
}

void EnvironmentAssets::placeRocks() {
    // Τοποθετεί τυχαίους βράχους
    // Περίπου 20 βράχοι
    for (int i = 0; i < 20; i++) {
        float x = getRandomX();
        float y = getRandomY();
        
        EnvironmentFactory::createRock(*manager, x, y);
    }
}

float EnvironmentAssets::getRandomX() {
    std::uniform_real_distribution<float> dist(0.0f, static_cast<float>(worldWidth * tileSize));
    return dist(rng);
}

float EnvironmentAssets::getRandomY() {
    std::uniform_real_distribution<float> dist(0.0f, static_cast<float>(worldHeight * tileSize));
    return dist(rng);
}

int EnvironmentAssets::getRandomGrassType() {
    std::uniform_int_distribution<int> dist(1, 4);  // grass1 to grass4
    return dist(rng);
}
