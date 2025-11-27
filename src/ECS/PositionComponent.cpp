#include <ECS/PositionComponent.hpp>
#include <Game.hpp>
#include <Collision.hpp>
#include <ECS/ColliderComponent.hpp>

void PositionComponent::update() {
    // Αποθηκεύει την παλιά θέση
    float oldX = position.x;
    float oldY = position.y;
    
    // Εφαρμόζει κίνηση
    position.x += velocity.x * speed;
    position.y += velocity.y * speed;
    
    // Ελέγχει σύγκρουση με πλακίδια και επαναφέρει αν υπάρχει σύγκρουση με στερεά πλακίδια
    if (Game::map && Collision::checkTileCollision(position.x, position.y, Game::map)) {
        if (Game::debugMode) std::cout << "Tile collision at (" << position.x << ", " << position.y << ") - reverting" << std::endl;
        position.x = oldX;
        position.y = oldY;
        return; // πρόωρη έξοδος αν σύγκρουση με πλακίδιο
    }

    // Check collision with entity colliders (rocks, trees, etc.)
    if (Game::managerPtr) {
<<<<<<< HEAD
        SDL_Rect playerCollider;
        // Το collider του παίκτη ταιριάζει με το μέγεθος του εμφανιζόμενου sprite
        playerCollider.x = static_cast<int>(position.x);
        playerCollider.y = static_cast<int>(position.y);
        playerCollider.w = width * scale;
        playerCollider.h = height * scale;
=======
        SDL_FRect playerCollider;
        // Το collider του παίκτη ταιριάζει με το μέγεθος του εμφανιζόμενου sprite
        playerCollider.x = position.x;
        playerCollider.y = position.y;
        playerCollider.w = (float)(width * scale);
        playerCollider.h = (float)(height * scale);
>>>>>>> SDL3

        for (const auto& ePtr : Game::managerPtr->getEntities()) {
            Entity* other = ePtr.get();
            if (other == entity) continue; // skip self
            if (other->hasComponent<ColliderComponent>()) {
                auto& cc = other->getComponent<ColliderComponent>();
                // Για αντικείμενα με μάσκα, χρησιμοποιεί pixel-perfect σύγκριση αν υπάρχει
                if (cc.mask.width > 0) {
                    // Pixel-perfect σύγκρουση: ελέγχει την επικάλυψη μεταξύ παίκτη και της άλλης μάσκας
<<<<<<< HEAD
                    SDL_Rect otherRect = cc.collider;
                    SDL_Rect intersect;
                    if (SDL_IntersectRect(&playerCollider, &otherRect, &intersect)) {
=======
                    SDL_FRect otherRect = cc.collider;
                    SDL_FRect intersect;
                    if (SDL_GetRectIntersectionFloat(&playerCollider, &otherRect, &intersect)) {
>>>>>>> SDL3
                        // Get player's collider component and mask
                        ColliderComponent* playerCC = nullptr;
                        if (entity->hasComponent<ColliderComponent>()) {
                            playerCC = &entity->getComponent<ColliderComponent>();
                        }
                        bool hit = false;
                        // Λαμβάνει υπόψη κλίμακα και μετατοπίσεις για τις δύο μάσκες
                        int otherScale = cc.position ? cc.position->scale : 1;
                        int playerScale = scale;
                        for (int y = 0; y < intersect.h && !hit; ++y) {
                            for (int x = 0; x < intersect.w; ++x) {
                                // Μετατροπή pixel κόσμου σε pixel μάσκας
                                // Πρώτα μετατρέπουμε σε συντεταγμένες εντός του collider, μετά σε unscaled, μετά αφαιρούμε το offset
                                int ox = (((intersect.x - otherRect.x) + x) / otherScale) - cc.mask.offsetX;
                                int oy = (((intersect.y - otherRect.y) + y) / otherScale) - cc.mask.offsetY;
                                int px = (((intersect.x - playerCollider.x) + x) / playerScale) - (playerCC ? playerCC->mask.offsetX : 0);
                                int py = (((intersect.y - playerCollider.y) + y) / playerScale) - (playerCC ? playerCC->mask.offsetY : 0);
                                bool otherSolid = cc.mask.isSolid(ox, oy);
                                bool playerSolid = playerCC && playerCC->mask.width > 0 ? playerCC->mask.isSolid(px, py) : true;
                                if (otherSolid && playerSolid) {
                                    hit = true;
                                    break;
                                }
                            }
                        }
                        if (hit) {
                            if (Game::debugMode) std::cout << "Blocked by '" << cc.tag << "' at entity rect (" << otherRect.x << "," << otherRect.y << "," << otherRect.w << "," << otherRect.h << ")" << std::endl;
                            position.x = oldX;
                            position.y = oldY;
                            return; // stop further checks
                        }
                    }
                } else {
                    // Προεπιλογή: ορθογώνια σύγκρουση (rectangle)
                    if (Collision::AABB(playerCollider, cc.collider)) {
                        if (Game::debugMode) std::cout << "Blocked by (rect) '" << cc.tag << "'" << std::endl;
                        position.x = oldX;
                        position.y = oldY;
                        return;
                    }
                }
            }
        }
    }

    // If we reach here, movement succeeded
    if (oldX != position.x || oldY != position.y) {
        if (Game::debugMode) std::cout << "Moved to (" << position.x << ", " << position.y << ")" << std::endl;
    }
}
