# CMakeLists.txt for Android Native Build
# This builds the game as a shared library (libgame.so) for Android

cmake_minimum_required(VERSION 3.22.1)
project("game" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 paths - downloaded during build
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL3")
set(SDL3_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_image")

# Game source directory (relative to this CMakeLists.txt)
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src")
set(GAME_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../include")

# Include directories
include_directories(
    ${GAME_INCLUDE_DIR}
    ${GAME_SRC_DIR}
    ${SDL3_DIR}/include
    ${SDL3_IMAGE_DIR}/include
)

# Game source files
# NOTE: Update these paths if your source structure differs
set(GAME_SOURCES
    ${GAME_SRC_DIR}/Game.cpp
    ${GAME_SRC_DIR}/TextureManager.cpp
    ${GAME_SRC_DIR}/HUD_new.cpp
    ${GAME_SRC_DIR}/GameObject.cpp
    ${GAME_SRC_DIR}/Map.cpp
    ${GAME_SRC_DIR}/Vector2D.cpp
    ${GAME_SRC_DIR}/Collision.cpp
    ${GAME_SRC_DIR}/Camera.cpp
    ${GAME_SRC_DIR}/EnvironmentAssets.cpp
    ${GAME_SRC_DIR}/Environment/Environment.cpp
    ${GAME_SRC_DIR}/ECS/ParticleComponent.cpp
    ${GAME_SRC_DIR}/ECS/SpriteComponent.cpp
    ${GAME_SRC_DIR}/ECS/AnimationComponent.cpp
    ${GAME_SRC_DIR}/ECS/PositionComponent.cpp
    ${GAME_SRC_DIR}/stb_image_write_impl.cpp
)

# Android-specific main wrapper
set(ANDROID_WRAPPER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/android_main.cpp
)

# Find prebuilt SDL2 libraries for Android
add_library(SDL3 SHARED IMPORTED)
set_target_properties(SDL3 PROPERTIES
    IMPORTED_LOCATION "${SDL3_DIR}/lib/${ANDROID_ABI}/libSDL3.so"
)

add_library(SDL3_image SHARED IMPORTED)
set_target_properties(SDL3_image PROPERTIES
    IMPORTED_LOCATION "${SDL3_IMAGE_DIR}/lib/${ANDROID_ABI}/libSDL3_image.so"
)

# Build the game as a shared library
add_library(game SHARED
    ${GAME_SOURCES}
    ${ANDROID_WRAPPER_SOURCES}
)

# Link with SDL2 libraries and Android system libraries
target_link_libraries(game
    SDL3
    SDL3_image
    android
    log
    GLESv2
    EGL
)

# Compile definitions for Android
target_compile_definitions(game PRIVATE
    __ANDROID__
    ANDROID
)
