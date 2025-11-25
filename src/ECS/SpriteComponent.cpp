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
    texturePath = path;
}

void SpriteComponent::init() {
    position = &entity->getComponent<PositionComponent>();

    srcRect.x = srcRect.y = 0;
    srcRect.w = position->width;
    srcRect.h = position->height;
    // Try automatic anchor detection from the sprite image (lowest opaque pixel row).
    if (!texturePath.empty()) {
        // Check for explicit override first
        int overrideRow = TextureManager::GetAnchorOverride(texturePath.c_str());
        if (overrideRow >= 0) {
            anchorY = overrideRow * position->scale;
        } else {
            int detected = TextureManager::DetectBottomOpaqueRow(texturePath.c_str());
            if (detected >= 0) {
                anchorY = detected * position->scale;
            } else {
                anchorY = position->height * position->scale;
            }
        }
    } else {
        anchorY = position->height * position->scale;
    }
}

void SpriteComponent::update() {
    // Μετατρέπει τη θέση από κόσμο σε θέση οθόνης χρησιμοποιώντας την camera
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

    // Debug: draw anchor/feet marker when game debug mode is enabled
    if (Game::debugMode && Game::renderer) {
        // Save previous draw color
        Uint8 pr, pg, pb, pa;
        SDL_GetRenderDrawColor(Game::renderer, &pr, &pg, &pb, &pa);

        // Red line at the computed anchor (feet)
        SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 255);
        int anchorScreenY = destRect.y + anchorY;
        SDL_RenderDrawLine(Game::renderer, destRect.x, anchorScreenY, destRect.x + destRect.w, anchorScreenY);
        // Small filled rectangle at center-bottom to mark exact point
        SDL_Rect mark{ destRect.x + destRect.w / 2 - 2, anchorScreenY - 2, 4, 4 };
        SDL_RenderFillRect(Game::renderer, &mark);

        // Restore previous color
        SDL_SetRenderDrawColor(Game::renderer, pr, pg, pb, pa);
    }
}
