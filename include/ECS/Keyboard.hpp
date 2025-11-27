#ifndef Keyboard_hpp
#define Keyboard_hpp

#include <Game.hpp>
#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
#include <ECS/AnimationComponent.hpp>
#include <TextureManager.hpp>
#include <ECS/ParticleComponent.hpp>

class Keyboard : public Component {
    public:
      PositionComponent *position;
      AnimationComponent *animation;
    Uint32 lastParticleTime = 0;

      void init() override {
        position = &entity->getComponent<PositionComponent>();
        // Εξασφαλίζει ότι το AnimationComponent υπάρχει πριν το αποκτήσει, αν και η σειρά στο Game.cpp το
        // διασφαλίζει
        if (entity->hasComponent<AnimationComponent>()) {
            animation = &entity->getComponent<AnimationComponent>();
        } else {
            animation = nullptr;
        }
      }

      void update() override {
        // Reset velocity each frame
        position->velocity.x = 0;
        position->velocity.y = 0;

        // Track whether the player is moving
        bool isMoving = false;

#ifdef __ANDROID__
        // On Android use movementDX/movementDY from Game (multi-touch aware)
        if (Game::movementActive) {
            const float dead = 0.25f; // normalized deadzone
            if (Game::movementDX > dead) { position->velocity.x = 1; isMoving = true; }
            else if (Game::movementDX < -dead) { position->velocity.x = -1; isMoving = true; }
            if (Game::movementDY > dead) { position->velocity.y = 1; isMoving = true; }
            else if (Game::movementDY < -dead) { position->velocity.y = -1; isMoving = true; }
            if (animation) animation->setFlip(Game::movementDX < 0.0f);
        }
        if (Game::attackActive) {
            if (animation && !animation->isBusy()) animation->play("Attack", false);
        }
    #else
        // Use keyboard on non-Android platforms
        const bool* keyState = SDL_GetKeyboardState(NULL);
        if (keyState[SDL_SCANCODE_W]) { position->velocity.y = -1; isMoving = true; }
        if (keyState[SDL_SCANCODE_S]) { position->velocity.y = 1; isMoving = true; }
        if (keyState[SDL_SCANCODE_A]) { position->velocity.x = -1; isMoving = true; }
        if (keyState[SDL_SCANCODE_D]) { position->velocity.x = 1; isMoving = true; }
#endif
        
        // Ενημερώνει το animation με βάση την κατάσταση κίνησης, αλλά δεν διακόπτει μη-επαναλαμβανόμενες animations
        if (animation) {
            if (!animation->isBusy()) {
                if (isMoving) {
                    if (animation->getCurrentAnimation() != "Walk") animation->play("Walk");
                } else {
                    if (animation->getCurrentAnimation() != "Idle") animation->play("Idle");
                }
            }
        }
        // On non-Android the direction is determined from keyboard
        // (on Android we already set flip during touch handling above)
        if (animation) {
#ifndef __ANDROID__
            const bool* keyState2 = SDL_GetKeyboardState(NULL);
            if (keyState2[SDL_SCANCODE_A]) { animation->setFlip(true); }
            else if (keyState2[SDL_SCANCODE_D]) { animation->setFlip(false); }
#endif
            // Attack is handled either by Game event loop (keyboard) or via touch above
        }
        // Spawn dust particles when moving (every ~80ms)
        if (isMoving && Game::managerPtr) {
            Uint32 now = SDL_GetTicks();
            if (now - lastParticleTime > 80) {
                lastParticleTime = now;
                // spawn behind player relative to velocity (base offsets)
                float ox = -position->velocity.x * 8.0f;
                float oy = -position->velocity.y * 4.0f;
                auto &e = Game::managerPtr->addEntity();
                // Small particle size; position uses world coords
                // Compute spawn at player's feet (centered horizontally) so particles appear behind/under the player
                int pW = position->width * position->scale;
                int partW = 8 * 1; // particle width * scale (hardcoded as constructed below)
                int partH = 8 * 1;
                // Try to detect player's visual feet (anchor) from current animation frame if available
                int anchorRow = -1;
                if (animation) {
                    std::string frame = animation->getCurrentFramePath();
                    if (!frame.empty()) {
                        int ov = TextureManager::GetAnchorOverride(frame.c_str());
                        if (ov >= 0) anchorRow = ov;
                        else {
                            int det = TextureManager::DetectBottomOpaqueRow(frame.c_str());
                            if (det >= 0) anchorRow = det;
                        }
                    }
                }
                if (anchorRow < 0) anchorRow = position->height; // fallback to full height

                float feetWorldY = position->position.y + (anchorRow * position->scale);
                // Add some horizontal and vertical randomness so particles look organic
                int vRange = 8; // pixels up/down (increased for more variety)
                int hRange = 4; // pixels left/right
                float randY = static_cast<float>((rand() % (vRange * 2 + 1)) - vRange);
                float randX = static_cast<float>((rand() % (hRange * 2 + 1)) - hRange);

                float spawnX = position->position.x + (pW / 2.0f) - (partW / 2.0f) + ox + randX;
                float spawnY = feetWorldY - partH + oy + randY; // align particle bottom with player's detected feet + randomness
                e.addComponent<PositionComponent>(spawnX, spawnY, partH, partW, 1);
                e.addComponent<SpriteComponent>("sprites/particles/dust_particles_small.png");
                // Make particle render behind player by forcing anchor above the sprite
                // Use negative anchor equal to particle height*scale so drawOrder is above player's feet
                auto &pPos = e.getComponent<PositionComponent>();
                int behindAnchor = - (pPos.height * pPos.scale);
                e.getComponent<SpriteComponent>().setAnchorY(behindAnchor);
                // Small velocity opposite of movement with randomness
                float vx = (-position->velocity.x) * (0.4f + (rand() % 20) / 100.0f);
                float vy = (-position->velocity.y) * (0.2f + (rand() % 20) / 200.0f);
                e.addComponent<ParticleComponent>(400.0f, vx, vy, 0.9f);
            }
        }
      }
};

#endif
