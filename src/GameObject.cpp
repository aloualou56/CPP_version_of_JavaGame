#include <GameObject.hpp>
#include <TextureManager.hpp>

GameObject::GameObject(const char* texturesheet, int x, int y) {
    
    objTexture = TextureManager::LoadTexture(texturesheet);

    xpos = x;
    ypos = y;
}

void GameObject::Update() {



    srcRect.h = 48.0f;
    srcRect.w = 48.0f;
    srcRect.x = 0.0f;
    srcRect.y = 0.0f;

    destRect.x = (float)xpos;
    destRect.y = (float)ypos;
    destRect.w = srcRect.w;
    destRect.h = srcRect.h * 2.0f;

}

void GameObject::Render() {
    
    SDL_RenderTexture(Game::renderer, objTexture, &srcRect, &destRect);
}