#ifndef PickupComponent_hpp
#define PickupComponent_hpp

#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>

// Handles the "touch it and something happens" half of item interaction
// (key/door). Blocking for solid items (the door) is handled separately by
// the normal ColliderComponent path - this only handles the pickup/unlock
// effect itself, mirroring Java's Player.pickUpObject() switch on key/door.
class PickupComponent : public Component {
public:
    enum class Type { Key, Door };

    PickupComponent(Type t, float proximityMargin = 0.0f) : type(t), margin(proximityMargin) {}

    void init() override;
    void update() override;

private:
    Type type;
    float margin;
    PositionComponent* position = nullptr;
};

#endif
