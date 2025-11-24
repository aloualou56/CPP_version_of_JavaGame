#include "TextureManager.hpp"

SDL_Texture* TextureManager::LoadTexture(const char* texture) {

    SDL_Surface* tempSurface = IMG_Load(texture);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    return tex;
}

// Συμβατότητα προς τα πίσω: υπερφόρτωση 3 ορισμάτων (κάποια μεταγλωττισμένα αντικείμενα μπορεί
// να αναφέρονται σε αυτό το σύμβολο)
void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest) {
    TextureManager::Draw(tex, src, dest, SDL_FLIP_NONE);
}

void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip) {
    // Χρήση SDL_RenderCopyEx για να επιτρέπεται ο οριζόντιος κατοπτρισμός (flip) του χαρακτήρα
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0.0, nullptr, flip);
}