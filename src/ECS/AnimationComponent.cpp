#include "AnimationComponent.hpp"
#include "PositionComponent.hpp"
#include "../TextureManager.hpp"
#include "../Game.hpp"
#include "../Camera.hpp"
#include "ECS.hpp"

AnimationComponent::~AnimationComponent() {
    for (auto& anim : animations) {
        for (auto* tex : anim.second) {
            SDL_DestroyTexture(tex);
        }
    }
    animations.clear();
}

void AnimationComponent::init() {
    position = &entity->getComponent<PositionComponent>();
    
    srcRect.x = srcRect.y = 0;
    srcRect.w = position->width;
    srcRect.h = position->height;
}

void AnimationComponent::addAnimation(const std::string& name, const std::vector<std::string>& filePaths, int speed) {
    std::vector<SDL_Texture*> textures;
    for (const auto& path : filePaths) {
        SDL_Texture* tex = TextureManager::LoadTexture(path.c_str());
        if (tex) {
            textures.push_back(tex);
        }
    }

    if (!textures.empty()) {
        animations[name] = textures;
        animationSpeeds[name] = speed;

        // If this is the first animation, set it as default
        if (currentAnimation.empty()) {
            currentAnimation = name;
            animIndex = 0;
            animSpeed = speed;
            animated = true;
        }
    }
}

void AnimationComponent::play(const std::string& animName) {
    if (currentAnimation != animName && animations.count(animName) > 0) {
        currentAnimation = animName;
        animIndex = 0;
        animSpeed = animationSpeeds[animName];
        lastFrameTime = SDL_GetTicks();
    }
}

void AnimationComponent::update() {
    if (animated && !currentAnimation.empty()) {
        if (SDL_GetTicks() - lastFrameTime > static_cast<Uint32>(animSpeed)) {
            animIndex++;
            if (animIndex >= animations[currentAnimation].size()) {
                animIndex = 0;
            }
            lastFrameTime = SDL_GetTicks();
        }
    }
    
    // Update srcRect to match current texture size
    if (!currentAnimation.empty() && !animations[currentAnimation].empty()) {
        SDL_Texture* currentTex = animations[currentAnimation][animIndex];
        if (currentTex) {
            SDL_QueryTexture(currentTex, NULL, NULL, &srcRect.w, &srcRect.h);
            srcRect.x = 0;
            srcRect.y = 0;
        }
    }

    // We update destRect based on position and camera
    if (Game::camera) {
        destRect.x = Game::camera->worldToScreenX(position->position.x);
        destRect.y = Game::camera->worldToScreenY(position->position.y);
    } else {
        destRect.x = static_cast<int>(position->position.x);
        destRect.y = static_cast<int>(position->position.y);
    }
    destRect.w = position->width * position->scale;
    destRect.h = position->height * position->scale;

    // We assume the texture size matches the component size or we just draw the whole texture.
    // For this specific case, we'll query the texture to be safe, or just use NULL for srcRect to draw full texture.
    // Using NULL for srcRect in SDL_RenderCopy draws the entire texture.
}

void AnimationComponent::draw() {
    if (!currentAnimation.empty() && !animations[currentAnimation].empty()) {
        SDL_Texture* currentTex = animations[currentAnimation][animIndex];
        // Passing NULL for srcRect to draw the entire texture into destRect
        TextureManager::Draw(currentTex, srcRect, destRect);
    }
}
