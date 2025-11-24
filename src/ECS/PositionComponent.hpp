#ifndef PositionComponent_hpp
#define PositionComponent_hpp

#include "ECS.hpp"
#include "../Vector2D.hpp"

class PositionComponent : public Component {

    

    public:
      Vector2D position;
      Vector2D velocity;

      int height = 48;
      int width = 48;
      int scale = 1;

      int speed = 4;

      PositionComponent() {
        position.Zero();
      }

      PositionComponent(float x, float y) {
         position.Zero();
      }

      PositionComponent(int sc) {
        position.Zero();
        scale = sc;
      }

      PositionComponent(float x, float y, int h, int w, int sc) {
        position.x = x;
        position.y = y;
        width = w;
        height = h;
        scale = sc;
      }

      void init() override {
        velocity.Zero();
      }

      void update() override {
        position.x += velocity.x * speed;
        position.y += velocity.y * speed;
     
      }

      void setPos(int x, int y) {
     
      }
};



#endif