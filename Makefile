# Simple Makefile for building on Windows with MinGW (SDL3)
# Assumes SDL3 and SDL3_image installed under C:/SDL3 and C:/SDL3_image respectively

CXX ?= x86_64-w64-mingw32-g++

SDL_INCLUDE_PATH = C:/SDL3/include
SDL_LIB_PATH     = C:/SDL3/lib
SDL_IMAGE_INCLUDE = C:/SDL3_image/include
SDL_IMAGE_LIB     = C:/SDL3_image/lib

CXXFLAGS = -std=c++17 -Wall -Wextra -g -I$(SDL_INCLUDE_PATH) -I$(SDL_INCLUDE_PATH)/SDL3 -I$(SDL_IMAGE_INCLUDE) -Iinclude
SDL_LIBS = -L$(SDL_LIB_PATH) -L$(SDL_IMAGE_LIB) -lmingw32 -lSDL3 -lSDL3_image

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = .
TARGET  = Game.exe

SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Game.cpp \
          $(SRC_DIR)/TextureManager.cpp \
          $(SRC_DIR)/HUD_new.cpp \
          $(SRC_DIR)/GameObject.cpp \
          $(SRC_DIR)/Map.cpp \
          $(SRC_DIR)/Vector2D.cpp \
          $(SRC_DIR)/Collision.cpp \
          $(SRC_DIR)/Camera.cpp \
          $(SRC_DIR)/EnvironmentAssets.cpp \
          $(SRC_DIR)/Environment/Environment.cpp \
          $(SRC_DIR)/ECS/ParticleComponent.cpp \
          $(SRC_DIR)/stb_image_write_impl.cpp

OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/Game.o \
          $(OBJ_DIR)/TextureManager.o \
          $(OBJ_DIR)/HUD_new.o \
          $(OBJ_DIR)/GameObject.o \
          $(OBJ_DIR)/Map.o \
          $(OBJ_DIR)/Vector2D.o \
          $(OBJ_DIR)/Collision.o \
          $(OBJ_DIR)/Camera.o \
          $(OBJ_DIR)/EnvironmentAssets.o \
          $(OBJ_DIR)/Environment.o \
          $(OBJ_DIR)/SpriteComponent.o \
          $(OBJ_DIR)/AnimationComponent.o \
          $(OBJ_DIR)/PositionComponent.o \
          $(OBJ_DIR)/ParticleComponent.o \
          $(OBJ_DIR)/stb_image_write_impl.o

all: $(TARGET)

$(OBJ_DIR):
	@if not exist  mkdir 

$(TARGET): $(OBJECTS)
	  -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	  -c $< -o $@

## Clean SDL3-ready Makefile (Windows friendly)

CXX ?= x86_64-w64-mingw32-g++

SDL_INCLUDE_PATH = C:/SDL3/include
SDL_LIB_PATH     = C:/SDL3/lib
SDL_IMAGE_INCLUDE = C:/SDL3_image/include
SDL_IMAGE_LIB     = C:/SDL3_image/lib

CXXFLAGS = -std=c++17 -Wall -Wextra -g -I$(SDL_INCLUDE_PATH) -I$(SDL_INCLUDE_PATH)/SDL3 -I$(SDL_IMAGE_INCLUDE) -Iinclude
SDL_LIBS = -L$(SDL_LIB_PATH) -L$(SDL_IMAGE_LIB) -lmingw32 -lSDL3 -lSDL3_image

SRC_DIR = src
OBJ_DIR = obj
TARGET  = Game.exe

SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Game.cpp \
          $(SRC_DIR)/TextureManager.cpp \
          $(SRC_DIR)/HUD_new.cpp \
          $(SRC_DIR)/GameObject.cpp \
          $(SRC_DIR)/Map.cpp \
          $(SRC_DIR)/Vector2D.cpp \
          $(SRC_DIR)/Collision.cpp \
          $(SRC_DIR)/Camera.cpp \
          $(SRC_DIR)/EnvironmentAssets.cpp \
          $(SRC_DIR)/Environment/Environment.cpp \
          $(SRC_DIR)/ECS/ParticleComponent.cpp \
          $(SRC_DIR)/stb_image_write_impl.cpp

OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/Game.o \
          $(OBJ_DIR)/TextureManager.o \
          $(OBJ_DIR)/HUD_new.o \
          $(OBJ_DIR)/GameObject.o \
          $(OBJ_DIR)/Map.o \
          $(OBJ_DIR)/Vector2D.o \
          $(OBJ_DIR)/Collision.o \
          $(OBJ_DIR)/Camera.o \
          $(OBJ_DIR)/EnvironmentAssets.o \
          $(OBJ_DIR)/Environment.o \
          $(OBJ_DIR)/SpriteComponent.o \
          $(OBJ_DIR)/AnimationComponent.o \
          $(OBJ_DIR)/PositionComponent.o \
          $(OBJ_DIR)/ParticleComponent.o \
          $(OBJ_DIR)/stb_image_write_impl.o

all: $(TARGET)

$(OBJ_DIR):
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(SDL_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Environment.o: $(SRC_DIR)/Environment/Environment.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/SpriteComponent.o: $(SRC_DIR)/ECS/SpriteComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/AnimationComponent.o: $(SRC_DIR)/ECS/AnimationComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/PositionComponent.o: $(SRC_DIR)/ECS/PositionComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/ParticleComponent.o: $(SRC_DIR)/ECS/ParticleComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(TARGET)" del /f /q "$(TARGET)"

rebuild: clean all

.PHONY: all clean rebuild
