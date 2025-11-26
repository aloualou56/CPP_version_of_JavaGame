#include <ECS/AnimationComponent.hpp>
#include <ECS/PositionComponent.hpp>
#include <TextureManager.hpp>
#include <Game.hpp>
#include <Camera.hpp>
#include <ECS/ECS.hpp>

AnimationComponent::~AnimationComponent() {
    // Textures are owned by TextureManager cache; do not destroy here.
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
        animationFilePaths[name] = filePaths;
            // Προεπιλεγμένη συμπεριφορά επανάληψης για αυτό το animation
        animationLooping[name] = true;

            // Εάν αυτή είναι η πρώτη animation, την ορίζει ως προεπιλογή
        if (currentAnimation.empty()) {
            currentAnimation = name;
            animIndex = 0;
            animSpeed = speed;
            animated = true;
        }
    }
}

std::string AnimationComponent::getCurrentFramePath() const {
    if (currentAnimation.empty()) return std::string();
    auto it = animationFilePaths.find(currentAnimation);
    if (it == animationFilePaths.end()) return std::string();
    if (animIndex < 0 || animIndex >= static_cast<int>(it->second.size())) return std::string();
    return it->second[animIndex];
}

std::string AnimationComponent::getFirstFramePath(const std::string& name) const {
    auto it = animationFilePaths.find(name);
    if (it == animationFilePaths.end() || it->second.empty()) return std::string();
    return it->second[0];
}

void AnimationComponent::play(const std::string& animName, bool loop) {
    // Αλλάζει animation μόνο όταν το ζητούμενο διαφέρει από το τρέχον
    if (currentAnimation != animName && animations.count(animName) > 0) {
        currentAnimation = animName;
        animIndex = 0;
        animSpeed = animationSpeeds[animName];
        lastFrameTime = SDL_GetTicks();
        animated = true;
        animationLooping[animName] = loop;
        if (Game::debugMode) std::cout << "Animation play: " << animName << " loop=" << (loop?"true":"false") << std::endl;
    }
}

void AnimationComponent::update() {
    if (animated && !currentAnimation.empty()) {
        if (SDL_GetTicks() - lastFrameTime > static_cast<Uint32>(animSpeed)) {
            animIndex++;
            size_t frameCount = animations[currentAnimation].size();
            if (animIndex >= static_cast<int>(frameCount)) {
                        // Αν η animation είναι σε loop, κάνει wrap; αλλιώς τελειώνει και ενδεχομένως επιστρέφει στο 'Idle'
                if (animationLooping[currentAnimation]) {
                    animIndex = 0;
                } else {
                            // Μη-επαναλαμβανόμενη τελείωσε. Προσπαθεί να επιστρέψει σε 'Idle' αν υπάρχει
                    if (animations.count("Idle") > 0) {
                        currentAnimation = "Idle";
                        animIndex = 0;
                        animSpeed = animationSpeeds["Idle"];
                        lastFrameTime = SDL_GetTicks();
                        animationLooping[currentAnimation] = true;
                    } else {
                        // Δεν υπάρχει ορισμένο 'Idle' animation: περιορίζει στο τελευταίο καρέ και σταματά το animation
                        animIndex = static_cast<int>(frameCount) - 1;
                        animated = false;
                    }
                }
            }
            // debug: εκτυπώνει τον δείκτη του καρέ όταν προχωρά
            if (Game::debugMode) std::cout << "Anim(" << currentAnimation << ") frame=" << animIndex << std::endl;
            lastFrameTime = SDL_GetTicks();
        }
    }
    
    // Ενημερώνει το srcRect ώστε να ταιριάζει με το μέγεθος της τρέχουσας υφής
    if (!currentAnimation.empty() && !animations[currentAnimation].empty()) {
        SDL_Texture* currentTex = animations[currentAnimation][animIndex];
        if (currentTex) {
            SDL_QueryTexture(currentTex, NULL, NULL, &srcRect.w, &srcRect.h);
            srcRect.x = 0;
            srcRect.y = 0;
        }
    }

    // Ενημερώνουμε το destRect με βάση τη θέση και την κάμερα
    if (Game::camera) {
        destRect.x = Game::camera->worldToScreenX(position->position.x);
        destRect.y = Game::camera->worldToScreenY(position->position.y);
    } else {
        destRect.x = static_cast<int>(position->position.x);
        destRect.y = static_cast<int>(position->position.y);
    }
    destRect.w = position->width * position->scale;
    destRect.h = position->height * position->scale;

    // Υποθέτουμε ότι το μέγεθος της υφής ταιριάζει με το μέγεθος του component ή σχεδιάζουμε ολόκληρη την υφή.
    // Σε αυτή την περίπτωση, μπορούμε να ρωτήσουμε το μέγεθος της υφής για ασφάλεια, ή απλά να χρησιμοποιήσουμε NULL για το srcRect.
    // Η χρήση NULL για srcRect στο SDL_RenderTexture σχεδιάζει ολόκληρη την υφή.
}

void AnimationComponent::draw() {
    if (!currentAnimation.empty() && !animations[currentAnimation].empty()) {
        SDL_Texture* currentTex = animations[currentAnimation][animIndex];
        // Draw using optional flip (horizontal) based on `flip` flag
        TextureManager::Draw(currentTex, srcRect, destRect, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}
