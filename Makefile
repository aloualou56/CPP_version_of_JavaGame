# ==========================================
# 1. COMPILER & PATHS CONFIGURATION
# ==========================================
CXX ?= g++

# You can override the compiler by setting the CXX environment variable, e.g.
#   set CXX=C:\msys64\mingw64\bin\g++.exe
# This keeps the default simple (`g++`) but allows tools or user to point
# to an absolute compiler path when needed.

<<<<<<< HEAD
# --- OPTION A: Standard MSYS2 (UCRT64) ---
# Most common if you followed modern tutorials
SDL_INCLUDE_PATH = C:/SDL2/include
SDL_LIB_PATH     = C:/SDL2/lib/x64

# --- OPTION B: Old MSYS2 (MINGW64) ---
=======
# --- OPTION A: Standard MSYS2 (UCRT64) for SDL3 ---
# Most common if you followed modern tutorials
SDL_INCLUDE_PATH = C:/SDL3/include
# Default to C:/SDL3/lib where the mingw import libs are usually located
SDL_LIB_PATH     = C:/SDL3/lib

# If you installed SDL_image separately, point here (example you provided)
SDL_IMAGE_INCLUDE = C:/SDL3_image/include
# Common layout: libraries placed under C:/SDL3_image/lib (or lib/x64). Use plain 'lib' by default.
SDL_IMAGE_LIB     = C:/SDL3_image/lib

# --- OPTION B: Old MSYS2 (MINGW64) for SDL3 ---
>>>>>>> SDL3
# Uncomment these two lines if Option A fails
# SDL_INCLUDE_PATH = C:/msys64/mingw64/include
# SDL_LIB_PATH     = C:/msys64/mingw64/lib

<<<<<<< HEAD
# --- OPTION C: Custom Install (e.g. C:/SDL2_Libs) ---
# SDL_INCLUDE_PATH = C:/SDL2_Libs/SDL2-2.30.9/x86_64-w64-mingw32/include
# SDL_LIB_PATH     = C:/SDL2_Libs/SDL2-2.30.9/x86_64-w64-mingw32/lib
=======
# --- OPTION C: Custom Install (e.g. C:/SDL3_Libs) ---
# SDL_INCLUDE_PATH = C:/SDL3_Libs/SDL3-3.x.x/x86_64-w64-mingw32/include
# SDL_LIB_PATH     = C:/SDL3_Libs/SDL3-3.x.x/x86_64-w64-mingw32/lib
>>>>>>> SDL3


# ==========================================
# 2. FLAGS & LIBRARIES
# ==========================================
<<<<<<< HEAD
# -Dmain=SDL_main is required for Windows
# -I points to the include folder so <SDL2/SDL.h> works
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Dmain=SDL_main -I$(SDL_INCLUDE_PATH) -I$(SDL_INCLUDE_PATH)/SDL2 -Iinclude

# Linker flags: Must include the library path (-L) and specific libraries (-l)
# Order matters: mingw32 -> SDL2main -> SDL2 -> SDL2_image
SDL_LIBS = -L$(SDL_LIB_PATH) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
=======
# -Dmain=SDL_main is no longer required for SDL3 on Windows
# -I points to the include folder so <SDL3/SDL.h> works
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I$(SDL_INCLUDE_PATH) -I$(SDL_INCLUDE_PATH)/SDL3 -I$(SDL_IMAGE_INCLUDE) -Iinclude

# Linker flags: Must include the library path (-L) and specific libraries (-l)
# SDL3 uses: SDL3 and SDL3_image (no separate main library)
SDL_LIBS = -L$(SDL_LIB_PATH) -L$(SDL_IMAGE_LIB) -lmingw32 -lSDL3 -lSDL3_image

# At runtime, ensure the SDL3 and SDL3_image DLLs are discoverable, e.g. copy
# C:/SDL3/bin/* and C:/SDL3_image/bin/* into the folder with Game.exe or add
# those paths to your PATH environment variable when running the game.
>>>>>>> SDL3


# ==========================================
# 3. DIRECTORIES & FILES
# ==========================================
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = .
TARGET  = $(BIN_DIR)/Game.exe

# Source files list
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

# Object files list (Flattened structure)
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


# ==========================================
# 4. BUILD RULES
# ==========================================

# Default target
all: $(TARGET)

# Create object directory (Windows command)
$(OBJ_DIR):
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(SDL_LIBS)

# --- Compilation Rules ---

# Generic rule for src/*.cpp
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Specific rules for subdirectories (Flattening to obj/ folder)
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

# --- Clean Rule (Windows Native) ---
clean:
<<<<<<< HEAD
	@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	@if exist $(TARGET) del /f /q $(TARGET)
=======
	-@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	-@rm -rf $(OBJ_DIR)
	-@if exist $(TARGET) del /f /q $(TARGET)
	-@rm -f $(TARGET)
>>>>>>> SDL3

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild