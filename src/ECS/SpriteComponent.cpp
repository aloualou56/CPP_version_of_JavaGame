#include "SpriteComponent.hpp"
#include "PositionComponent.hpp"
#include "../TextureManager.hpp"
#include "../Game.hpp"
#include "../Camera.hpp"
#include "ECS.hpp"

SpriteComponent::SpriteComponent(const char* path) {
    setTex(path);
    std::cout << "loaded " << path << std::endl;
}

SpriteComponent::~SpriteComponent() {
    SDL_DestroyTexture(texture);
}

void SpriteComponent::setTex(const char* path) {
    texture = TextureManager::LoadTexture(path);
}

void SpriteComponent::init() {
    position = &entity->getComponent<PositionComponent>();

    srcRect.x = srcRect.y = 0;
    srcRect.w = position->width;
    srcRect.h = position->height;
}

void SpriteComponent::update() {
    // Convert world position to screen position using camera
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

void SpriteComponent::draw() {
    TextureManager::Draw(texture, srcRect, destRect);
}
