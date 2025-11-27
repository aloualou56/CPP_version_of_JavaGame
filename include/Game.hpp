#ifndef Game_hpp
#define Game_hpp

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "ECS/ECS.hpp"

class HUD;

class Camera;
class Map;

class Game {
    public:
      Game();
      ~Game();

      void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);

      void handleEvents();
      void update();
      void render();
      void clean();

      bool running() {return isRunning;}

      static SDL_Renderer *renderer;
      static SDL_Event event;
      static Camera* camera;
      static Map* map;
      // screen size (set on init)
      static int screenWidth;
      static int screenHeight;
      // Simple touch/multi-touch state for Android controls
      static long long movementFingerId; // finger id tracking movement (-1 = none)
      static long long attackFingerId;   // finger id tracking attack (-1 = none)
      static bool movementActive;
      static float movementDX; // -1..1
      static float movementDY; // -1..1
      static bool attackActive;
      // Haptic device (optional)
      static SDL_Haptic *haptic;
      // Εκθέτει τον παγκόσμιο manager που ορίζεται στο `Game.cpp`
      static Manager* managerPtr;
      // Παγκόσμιος διακόπτης debug για εμφάνιση/απόκρυψη οπτικών/καταγραφών debug
      static bool debugMode;

    private:
      int cnt = 0;
      bool isRunning;
      SDL_Window *window;
      // HUD and health are managed via ECS `HealthComponent` now
      HUD* hud = nullptr;
      unsigned int lastDamageTime = 0; // ms
      
};



#endif 
