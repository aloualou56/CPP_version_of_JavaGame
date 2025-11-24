#ifndef AnimationComponent_hpp
#define AnimationComponent_hpp

#include "Componets.hpp"
#include "SDL.h"
#include <map>
#include <vector>
#include <string>

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
    
public:
    bool animated = true;
    
    AnimationComponent() = default;
    
    ~AnimationComponent() {
        if (spriteSheet) {
            SDL_DestroyTexture(spriteSheet);
        }
    }
    
    void init() override {
        position = &entity->getComponent<PositionComponent>();
        
        srcRect.x = srcRect.y = 0;
        srcRect.w = position->width;
        srcRect.h = position->height;
    }
    
    void addAnimation(const std::string& name, int index, int frames, int speed) {
        animations.emplace(name, Animation(index, frames, speed));
    }
    
    void play(const std::string& animName) {
        if (currentAnimation != animName && animations.find(animName) != animations.end()) {
            currentAnimation = animName;
            animIndex = animations[animName].index;
            animFrames = animations[animName].frames;
            animSpeed = animations[animName].speed;
            lastFrameTime = SDL_GetTicks();
        }
    }
    
    void setTexture(SDL_Texture* texture) {
        spriteSheet = texture;
    }
    
    void update() override {
        if (animated && animFrames > 1) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastFrameTime > static_cast<Uint32>(animSpeed)) {
                animIndex++;
                if (animIndex >= animations[currentAnimation].index + animFrames) {
                    animIndex = animations[currentAnimation].index;
                }
                lastFrameTime = currentTime;
            }
        }
        
        srcRect.x = srcRect.w * animIndex;
        srcRect.y = 0;
        
        destRect.x = static_cast<int>(position->position.x);
        destRect.y = static_cast<int>(position->position.y);
        destRect.w = position->width * position->scale;
        destRect.h = position->height * position->scale;
    }
    
    void draw() override {
        TextureManager::Draw(spriteSheet, srcRect, destRect);
    }
};

#endif
