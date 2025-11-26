#ifndef ColliderComponent_hpp
#define ColliderComponent_hpp

#include <string>
#include <SDL3/SDL.h>
#include <ECS/ECS.hpp>
#include <ECS/PositionComponent.hpp>
#include <ECS/SpriteComponent.hpp>
#include <ECS/AnimationComponent.hpp>
#include <ImageMask.hpp>
#include <Camera.hpp>

class ColliderComponent : public Component {
     public:
      SDL_Rect collider;
      std::string tag;

    // Αφαιρέθηκαν μετατοπίσεις: η μάσκα ευθυγραμμίζεται πάντα στο πάνω-αριστερό του collider

    ImageMask mask;

    bool debugDraw = true;

      PositionComponent *position;

      ColliderComponent(std::string t) {
        tag = t;
            // Προεπιλογή: καμία μάσκα δεν έχει φορτωθεί
      }

      void init() override {
          if(!entity->hasComponent<PositionComponent>()) {
                entity->addComponent<PositionComponent>();
          }
          position = &entity->getComponent<PositionComponent>();
        // Προτιμά να φορτώσει μάσκα από το sprite ή την animation της οντότητας αν είναι διαθέσιμη
        bool loaded = false;
        if (entity->hasComponent<SpriteComponent>()) {
            auto& sp = entity->getComponent<SpriteComponent>();
            const std::string& p = sp.getPath();
            if (!p.empty()) {
                loaded = mask.loadFromPNG(p);
            }
        }
        if (!loaded && entity->hasComponent<AnimationComponent>()) {
            auto& ac = entity->getComponent<AnimationComponent>();
            // Πρώτα δοκιμάζει το τρέχον καρέ, μετά το πρώτο καρέ του 'Idle' ως εφεδρικό
            std::string p = ac.getCurrentFramePath();
            if (p.empty()) p = ac.getFirstFramePath("Idle");
            if (!p.empty()) {
                loaded = mask.loadFromPNG(p);
            }
        }

        // Εφεδρικές επιλογές με βάση το tag ή προεπιλεγμένες εικόνες
        if (!loaded) {
             if (tag == "rock") {
                 mask.loadFromPNG("sprites/objects/rock.png");
             } else if (tag == "bush") {
                 mask.loadFromPNG("sprites/objects/thamnos_tonia.png");
             } else if (tag == "player") {
                 mask.loadFromPNG("sprites/characters/cutted-character/standing_sprites/standing_1.png");
             }
        }
      }

      void update() override {
        // Το collider καλύπτει ολόκληρη την περιοχή του εμφανιζόμενου sprite (προέλευση πάνω-αριστερά)
        collider.x = static_cast<int>(position->position.x);
        collider.y = static_cast<int>(position->position.y);
        collider.w = position->width * position->scale;
        collider.h = position->height * position->scale;
      }
    
      bool isDrawable() override { return true; }
      int drawOrder() override { return collider.y + collider.h; }

    void draw() override {
        if (Game::debugMode && Game::renderer) {
            // Μετατρέπει τις συντεταγμένες του collider από κόσμο σε οθόνη με την camera
            SDL_Rect screenRect = collider;
            if (Game::camera) {
                screenRect.x = Game::camera->worldToScreenX(collider.x);
                screenRect.y = Game::camera->worldToScreenY(collider.y);
            }
            // Σχεδιάζει το ορθογώνιο του collider με κόκκινο
            SDL_SetRenderDrawColor(Game::renderer, 255, 0, 0, 255);
            SDL_FRect screenRectF = { (float)screenRect.x, (float)screenRect.y, (float)screenRect.w, (float)screenRect.h };
            SDL_RenderRect(Game::renderer, &screenRectF);
            // Προαιρετικά, σχεδιάζει τα όρια της μάσκας με πράσινο (κλιμακωμένο)
            if (mask.width > 0 && mask.height > 0) {
                int sc = position ? position->scale : 1;
                SDL_FRect maskRectF = { (float)(screenRect.x + mask.offsetX * sc), (float)(screenRect.y + mask.offsetY * sc), (float)(mask.width * sc), (float)(mask.height * sc) };
                SDL_SetRenderDrawColor(Game::renderer, 0, 255, 0, 255);
                SDL_RenderRect(Game::renderer, &maskRectF);
            }
        }
    }
};

#endif
