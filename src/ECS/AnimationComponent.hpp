#ifndef AnimationComponent_hpp
#define AnimationComponent_hpp

#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "SDL.h"
#include <map>
#include <vector>
#include <string>

// Forward declarations
class TextureManager;

class AnimationComponent : public Component {
private:
    std::map<std::string, std::vector<SDL_Texture*>> animations;
    std::map<std::string, int> animationSpeeds;

    std::string currentAnimation;
    int animIndex = 0;
    int animSpeed = 100;
    
    SDL_Rect srcRect, destRect;
    PositionComponent* position;
    
    Uint32 lastFrameTime = 0;
    bool animated = false;
    bool flip = false;

public:
    AnimationComponent() = default;
    ~AnimationComponent();
    
    void init() override;
    void addAnimation(const std::string& name, const std::vector<std::string>& filePaths, int speed);
    void play(const std::string& animName);
    void update() override;
    void draw() override;
};

#endif
