#include "AnimationComponent.hpp"
#include "PositionComponent.hpp"
#include "../TextureManager.hpp"
#include "../Game.hpp"
#include "../Camera.hpp"
#include "ECS.hpp"

AnimationComponent::~AnimationComponent() {
    if (spriteSheet && ownsTexture) {
        SDL_DestroyTexture(spriteSheet);
    }
}

void AnimationComponent::init() {
    position = &entity->getComponent<PositionComponent>();
    
    srcRect.x = srcRect.y = 0;
    srcRect.w = position->width;
    srcRect.h = position->height;
}

void AnimationComponent::addAnimation(const std::string& name, int index, int frames, int speed) {
    animations.emplace(name, Animation(index, frames, speed));
}

void AnimationComponent::play(const std::string& animName) {
    if (currentAnimation != animName && animations.find(animName) != animations.end()) {
        currentAnimation = animName;
        animIndex = animations[animName].index;
        animFrames = animations[animName].frames;
        animSpeed = animations[animName].speed;
        lastFrameTime = SDL_GetTicks();
    }
}

void AnimationComponent::setTexture(SDL_Texture* texture, bool takeOwnership) {
    // Clean up old texture if we own it
    if (spriteSheet && ownsTexture) {
        SDL_DestroyTexture(spriteSheet);
    }
    spriteSheet = texture;
    ownsTexture = takeOwnership;
}

void AnimationComponent::update() {
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
    
    // Use camera coordinates like SpriteComponent
    if (Game::camera) {
        destRect.x = Game::camera->worldToScreenX(position->position.x);
        destRect.y = Game::camera->worldToScreenY(position->position.y);
    } else {
        destRect.x = static_cast<int>(position->position.x);
        destRect.y = static_cast<int>(position->position.y);
    }
    destRect.w = position->width * position->scale;
    destRect.h = position->height * position->scale;
}

void AnimationComponent::draw() {
    TextureManager::Draw(spriteSheet, srcRect, destRect);
}
