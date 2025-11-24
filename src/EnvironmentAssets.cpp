#include "EnvironmentAssets.hpp"
#include <iostream>

EnvironmentAssets::EnvironmentAssets(Manager* mgr, int worldW, int worldH, int tileS) 
    : manager(mgr), worldWidth(worldW), worldHeight(worldH), tileSize(tileS) {
    // Seed random number generator
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
    // Place random grass decorations across the map
    // About 100 grass decorations scattered around
    for (int i = 0; i < 100; i++) {
        float x = getRandomX();
        float y = getRandomY();
        int grassType = getRandomGrassType();
        
        EnvironmentFactory::createEnvGrass(*manager, grassType, x, y);
    }
}

void EnvironmentAssets::placeBushes() {
    // Place random bushes (thamnos tonia)
    // About 30 bushes
    for (int i = 0; i < 30; i++) {
        float x = getRandomX();
        float y = getRandomY();
        
        EnvironmentFactory::createThamnosTonia(*manager, x, y);
    }
}

void EnvironmentAssets::placeRocks() {
    // Place random rocks
    // About 20 rocks
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
