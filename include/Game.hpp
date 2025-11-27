#ifndef Game_hpp
#define Game_hpp

<<<<<<< HEAD
#include <SDL.h>
#include <SDL_image.h>
=======
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
>>>>>>> SDL3
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
