#ifndef InventoryComponent_hpp
#define InventoryComponent_hpp

#include <ECS/ECS.hpp>

// Player-only: minimal inventory state. Mirrors Java Player's `hasKey` int.
class InventoryComponent : public Component {
public:
    int keys = 0;
};

#endif
