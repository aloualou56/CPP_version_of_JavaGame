# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs) -lSDL2_image

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = .

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Game.cpp \
          $(SRC_DIR)/TextureManager.cpp \
          $(SRC_DIR)/GameObject.cpp \
          $(SRC_DIR)/Map.cpp \
          $(SRC_DIR)/Vector2D.cpp \
          $(SRC_DIR)/Collision.cpp \
          $(SRC_DIR)/Camera.cpp \
          $(SRC_DIR)/EnvironmentAssets.cpp \
          $(SRC_DIR)/Environment/Environment.cpp

# Object files
OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/Game.o \
          $(OBJ_DIR)/TextureManager.o \
          $(OBJ_DIR)/GameObject.o \
          $(OBJ_DIR)/Map.o \
          $(OBJ_DIR)/Vector2D.o \
          $(OBJ_DIR)/Collision.o \
          $(OBJ_DIR)/Camera.o \
          $(OBJ_DIR)/EnvironmentAssets.o \
          $(OBJ_DIR)/Environment.o \
          $(OBJ_DIR)/SpriteComponent.o \
          $(OBJ_DIR)/AnimationComponent.o \
          $(OBJ_DIR)/PositionComponent.o

# Target executable
TARGET = $(BIN_DIR)/Game

# Default target
all: $(TARGET)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(SDL_LIBS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

# Special rules for subdirectories
$(OBJ_DIR)/Environment.o: $(SRC_DIR)/Environment/Environment.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR)/SpriteComponent.o: $(SRC_DIR)/ECS/SpriteComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR)/AnimationComponent.o: $(SRC_DIR)/ECS/AnimationComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR)/PositionComponent.o: $(SRC_DIR)/ECS/PositionComponent.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
