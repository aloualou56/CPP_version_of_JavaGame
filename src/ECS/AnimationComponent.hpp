#ifndef AnimationComponent_hpp
#define AnimationComponent_hpp

#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "SDL.h"
#include <map>
#include <vector>
#include <string>

// Forward declarations for non-Component classes
class TextureManager;

struct Animation {
    int index;
    int frames;
    int speed;  // Frame delay in game ticks
    
    Animation() : index(0), frames(0), speed(100) {}
    Animation(int i, int f, int s) : index(i), frames(f), speed(s) {}
};

class AnimationComponent : public Component {
private:
    std::map<std::string, Animation> animations;
    int animIndex = 0;
    int animSpeed = 100;
    int animFrames = 1;
    
    SDL_Texture* spriteSheet;
    SDL_Rect srcRect, destRect;
    
    PositionComponent* position;
    
    std::string currentAnimation = "idle";
    Uint32 lastFrameTime = 0;
    bool ownsTexture = false;  // Track if we own the texture
    
public:
    bool animated = true;
    
    AnimationComponent() = default;
    ~AnimationComponent();
    
    void init() override;
    void addAnimation(const std::string& name, int index, int frames, int speed);
    void play(const std::string& animName);
    void setTexture(SDL_Texture* texture, bool takeOwnership = false);
    void update() override;
    void draw() override;
};

#endif
