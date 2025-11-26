#include <ECS/SpriteComponent.hpp>
#include <ECS/PositionComponent.hpp>
#include <TextureManager.hpp>
#include <Game.hpp>
#include <Camera.hpp>
#include <ECS/ECS.hpp>

SpriteComponent::SpriteComponent(const char* path) {
    setTex(path);
    std::cout << "loaded " << path << std::endl;
}

SpriteComponent::~SpriteComponent() {
    // Texture is owned by TextureManager cache; do not destroy here.
}

void SpriteComponent::setTex(const char* path) {
    texture = TextureManager::LoadTexture(path);
    texturePath = path;
}

void SpriteComponent::init() {
    position = &entity->getComponent<PositionComponent>();

    srcRect.x = srcRect.y = 0.0f;
    srcRect.w = (float)position->width;
    srcRect.h = (float)position->height;
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
        destRect.x = (float)Game::camera->worldToScreenX(position->position.x);
        destRect.y = (float)Game::camera->worldToScreenY(position->position.y);
    } else {
        destRect.x = position->position.x;
        destRect.y = position->position.y;
    }
    destRect.w = (float)(position->width * position->scale);
    destRect.h = (float)(position->height * position->scale);
}

void SpriteComponent::draw() {
    TextureManager::Draw(texture, srcRect, destRect);

    // Debug: draw anchor/feet marker when game debug mode is enabled
    if (Game::debugMode && Game::renderer) {
        // Save previous draw color (SDL_GetRenderDrawColor uses Uint8 channels)
        Uint8 pr, pg, pb, pa;
        SDL_GetRenderDrawColor(Game::renderer, &pr, &pg, &pb, &pa);

        // Red line at the computed anchor (feet)
        SDL_SetRenderDrawColor(Game::renderer, 1.0f, 0.0f, 0.0f, 1.0f);
        float anchorScreenY = destRect.y + anchorY;
        SDL_RenderLine(Game::renderer, destRect.x, anchorScreenY, destRect.x + destRect.w, anchorScreenY);
        // Small filled rectangle at center-bottom to mark exact point
        SDL_FRect mark{ destRect.x + destRect.w / 2.0f - 2.0f, anchorScreenY - 2.0f, 4.0f, 4.0f };
        SDL_RenderFillRect(Game::renderer, &mark);

        // Restore previous color (pass Uint8 channels)
        SDL_SetRenderDrawColor(Game::renderer, pr, pg, pb, pa);
    }
}
