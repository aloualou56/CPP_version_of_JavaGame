#include "Environment.hpp"
#include "../ECS/ECS.hpp"
#include "../ECS/Componets.hpp"

namespace EnvironmentFactory {
    
    Entity& createEnvGrass(Manager& manager, int grassType, float x, float y) {
        std::string spritePath = "sprites/tilesets/16x16 set/grass" + std::to_string(grassType) + ".png";
        
        auto& grass = manager.addEntity();
        grass.addComponent<PositionComponent>(x, y, 16, 16, 4);  // Scale 4x for smaller decorations
        grass.addComponent<SpriteComponent>(spritePath.c_str());
        
        return grass;
    }
    
    Entity& createThamnosTonia(Manager& manager, float x, float y) {
        auto& bush = manager.addEntity();
        bush.addComponent<PositionComponent>(x, y, 48, 48, 2);  // Scale 2x
        bush.addComponent<SpriteComponent>("sprites/objects/thamnos_tonia.png");
        bush.addComponent<ColliderComponent>("bush");
        
        return bush;
    }
    
    Entity& createTree(Manager& manager, float x, float y) {
        auto& tree = manager.addEntity();
        tree.addComponent<PositionComponent>(x, y, 48, 48, 3);  // Scale 3x
        tree.addComponent<SpriteComponent>("sprites/objects/tree.png");
        tree.addComponent<ColliderComponent>("tree");
        
        return tree;
    }
    
    Entity& createRock(Manager& manager, float x, float y) {
        auto& rock = manager.addEntity();
        rock.addComponent<PositionComponent>(x, y, 16, 16, 4);  // Scale 4x
        rock.addComponent<SpriteComponent>("sprites/objects/rock.png");
        rock.addComponent<ColliderComponent>("rock");
        
        return rock;
    }
}
