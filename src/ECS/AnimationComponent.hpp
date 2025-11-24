#ifndef AnimationComponent_hpp
#define AnimationComponent_hpp

#include "ECS.hpp"
#include "PositionComponent.hpp"
#include "SDL.h"
#include <map>
#include <vector>
#include <string>

// Προκαταρκτικές δηλώσεις
class TextureManager;

class AnimationComponent : public Component {
private:
    std::map<std::string, std::vector<SDL_Texture*>> animations;
    std::map<std::string, int> animationSpeeds;
    // Κρατά τα αρχικά μονοπάτια αρχείων για κάθε animation για να επιτρέπει το φόρτωμα μάσκας
    std::map<std::string, std::vector<std::string>> animationFilePaths;

    std::string currentAnimation;
    int animIndex = 0;
    int animSpeed = 100;
    
    SDL_Rect srcRect, destRect;
    PositionComponent* position;
    
    Uint32 lastFrameTime = 0;
    bool animated = false;
    bool flip = false;
    std::map<std::string, bool> animationLooping;

public:
    AnimationComponent() = default;
    ~AnimationComponent();
    
    void init() override;
    void addAnimation(const std::string& name, const std::vector<std::string>& filePaths, int speed);
    // Επιστρέφει το μονοπάτι αρχείου για το τρέχον καρέ του animation (ή κενή συμβολοσειρά)
    std::string getCurrentFramePath() const;
    // Επιστρέφει το μονοπάτι του πρώτου καρέ για ένα δεδομένο όνομα animation
    std::string getFirstFramePath(const std::string& name) const;
    void play(const std::string& animName, bool loop = true);
    void update() override;
    void draw() override;
    bool isDrawable() override { return true; }
    // Ταξινομεί με βάση το κάτω Y (πόδια) ώστε οι κινούμενες οντότητες να σχεδιάζονται σωστά σε σχέση με αντικείμενα
    int drawOrder() override { return destRect.y + destRect.h; }
    void setFlip(bool f) { flip = f; }
    // Επιστρέφει true όταν μια μη-επαναλαμβανόμενη animation παίζει την τρέχουσα στιγμή
    bool isBusy() const { 
        if (currentAnimation.empty()) return false;
        auto it = animationLooping.find(currentAnimation);
        if (it == animationLooping.end()) return false;
        return animated && !it->second;
    }
    // Επιστρέφει το όνομα της τρέχουσας animation
    const std::string& getCurrentAnimation() const { return currentAnimation; }
};

#endif
