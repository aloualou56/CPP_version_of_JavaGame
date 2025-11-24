#ifndef HealthComponent_hpp
#define HealthComponent_hpp

#include "ECS.hpp"

class HealthComponent : public Component {
public:
    float current = 0.0f;
    int maximum = 0;

    HealthComponent() = default;
    HealthComponent(int max) { maximum = max; current = (float)max; }

    void init() override {}

    void takeDamage(float v) {
        current -= v;
        if (current < 0.0f) current = 0.0f;
    }

    void heal(float v) {
        current += v;
        if (current > maximum) current = (float)maximum;
    }

    void set(float v) { current = v; if (current < 0.0f) current = 0.0f; if (current > maximum) current = (float)maximum; }
    float getCurrent() const { return current; }
    int getMax() const { return maximum; }
};

#endif
