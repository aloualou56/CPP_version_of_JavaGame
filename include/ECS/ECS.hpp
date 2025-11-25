#ifndef ECS_hpp
#define ECS_hpp

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>


class Component;
class Entity;

using ComponentID = std::size_t;

inline ComponentID getComponentTypeID() {
    
        static ComponentID lastID = 0;
        return lastID++;
}

template <typename T> inline ComponentID getComponentTypeID() noexcept {

        static_assert (std::is_base_of<Component, T>::value, "");
        static ComponentID typeID = getComponentTypeID();
        return typeID;
}

constexpr std::size_t maxComponents = 32;

using ComponentBitSet = std::bitset<maxComponents>;
using ComponentArray = std::array<Component*, maxComponents>;


class Component {

        public:
            Entity* entity;
    virtual void init() {}
    virtual void update() {}
    virtual void draw() {}
    // Αν αυτό το component πρέπει να λαμβάνεται υπόψη για τη σειρά σχεδίασης
    virtual bool isDrawable() { return false; }
    // Τάξη σχεδίασης / βάθος (μεγαλύτερο = σχεδιάζεται αργότερα / μπροστά)
    virtual int drawOrder() { return 0; }

            virtual ~Component() {}
};

class Entity {

        private:
            bool active = true;
            std::vector<std::unique_ptr<Component>> components;

            ComponentArray componentArray;
            ComponentBitSet componentBitSet;

        public:
            void update() {
                for(auto& c : components) c->update();
        
            }

            void draw() {
                for(auto& c : components) c->draw();
            }
            // Επιστρέφει δείκτες (raw pointers) σε components (χρησιμοποιείται από τον Manager για
            // ταξινομημένη σχεδίαση)
            std::vector<Component*> getComponentPointers() {
                std::vector<Component*> out;
                for (auto &u : components) out.push_back(u.get());
                return out;
            }
            bool isActive() const {return active;}
            void destroy() {active = false;}

            template <typename T> bool hasComponent() const {
                ComponentID componentID = getComponentTypeID<T>(); // πρόσθετος κώδικας — παράγει το id του component
                return componentBitSet[componentID];
            }

            template <typename T, typename... TArgs>
            T& addComponent(TArgs&&... mArgs) {

                T* c(new T(std::forward<TArgs>(mArgs)...));
                c->entity = this;
                std::unique_ptr<Component> uPtr{ c };
                components.emplace_back(std::move(uPtr));

                componentArray[getComponentTypeID<T>()] = c;
                componentBitSet[getComponentTypeID<T>()] = true;

                c->init();
                return *c;
            }


            template<typename T> T& getComponent() const {

                auto ptr(componentArray[getComponentTypeID<T>()]);
                return *static_cast<T*>(ptr);
            }
};

class Manager {
        private:
            std::vector<std::unique_ptr<Entity>> entities;


     public:
            void update() {
        
                for(auto& e : entities) e->update();
            }

            void draw() {

                // Συλλέγει τα components που είναι ζωγραφίσιμα από όλες τις οντότητες και τα ταξινομεί με βάση το drawOrder (y)
                std::vector<Component*> drawables;
                for(auto& e : entities) {
                    auto comps = e->getComponentPointers();
                    for (auto *c : comps) {
                        if (c && c->isDrawable()) drawables.push_back(c);
                    }
                }

                std::sort(drawables.begin(), drawables.end(), [](Component* a, Component* b) {
                    return a->drawOrder() < b->drawOrder();
                });

                for (auto *c : drawables) if (c) c->draw();
            }

            void refresh() {
        
                entities.erase(std::remove_if(std::begin(entities), std::end(entities),
                     [](const std::unique_ptr<Entity> &mEntity) {

                            return !mEntity->isActive();
                     }),
              
                            std::end(entities));
            }

            Entity& addEntity() {

                Entity* e = new Entity();
                std::unique_ptr<Entity> uPtr{ e };
                entities.emplace_back(std::move(uPtr));
                return *e;
            }

            // Εκθέτει τις οντότητες για ελέγχους σύγκρουσης και εξωτερική επανάληψη
            const std::vector<std::unique_ptr<Entity>>& getEntities() const { return entities; }
};

#endif
