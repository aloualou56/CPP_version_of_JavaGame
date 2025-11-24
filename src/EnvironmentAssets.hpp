#ifndef EnvironmentAssets_hpp
#define EnvironmentAssets_hpp

#include "ECS/ECS.hpp"
#include "Environment/Environment.hpp"
#include <random>
#include <ctime>

class EnvironmentAssets {
private:
    Manager* manager;
    std::mt19937 rng;
    
    int worldWidth;
    int worldHeight;
    int tileSize;
    
public:
    EnvironmentAssets(Manager* mgr, int worldW, int worldH, int tileS);
    ~EnvironmentAssets();
    
    void generateEnvironment();
    
private:
    void placeGrassDecorations();
    void placeBushes();
    void placeRocks();
    
    // Βοηθητική συνάρτηση για λήψη τυχαίας θέσης
    float getRandomX();
    float getRandomY();
    int getRandomGrassType();
};

#endif
