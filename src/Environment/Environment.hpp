#ifndef Environment_hpp
#define Environment_hpp

#include <string>

// Forward declarations
class Manager;
class Entity;

// Helper functions to create environment entities
namespace EnvironmentFactory {
    
    // Create a grass decoration entity
    Entity& createEnvGrass(Manager& manager, int grassType, float x, float y);
    
    // Create a thamnos tonia bush entity
    Entity& createThamnosTonia(Manager& manager, float x, float y);
    
    // Create a tree entity
    Entity& createTree(Manager& manager, float x, float y);
    
    // Create a rock entity
    Entity& createRock(Manager& manager, float x, float y);
}

#endif
