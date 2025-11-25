#ifndef Environment_hpp
#define Environment_hpp

#include <string>

// Προκαταρκτικές δηλώσεις
class Manager;
class Entity;

// Helper functions to create environment entities
namespace EnvironmentFactory {
    
    // Δημιουργεί μια οντότητα διακόσμησης χόρτου
    Entity& createEnvGrass(Manager& manager, int grassType, float x, float y);
    
    // Δημιουργεί μια οντότητα θάμνου (thamnos tonia)
    Entity& createThamnosTonia(Manager& manager, float x, float y);
    
    // Δημιουργεί μια οντότητα δέντρου
    Entity& createTree(Manager& manager, float x, float y);
    
    // Δημιουργεί μια οντότητα βράχου
    Entity& createRock(Manager& manager, float x, float y);
}

#endif
