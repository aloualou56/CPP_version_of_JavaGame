# CMakeLists.txt for Android Native Build
# This builds the game as a shared library (libgame.so) for Android

cmake_minimum_required(VERSION 3.22.1)
project("game" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 paths - downloaded during build
set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL2")
set(SDL2_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_image")

# Game source directory (relative to this CMakeLists.txt)
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src")
set(GAME_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../include")

# Include directories
include_directories(
    ${GAME_INCLUDE_DIR}
    ${GAME_SRC_DIR}
    ${SDL2_DIR}/include
    ${SDL2_IMAGE_DIR}/include
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
add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_DIR}/lib/${ANDROID_ABI}/libSDL2.so"
)

# SDL2main is optional on Android - only include if it exists
if(EXISTS "${SDL2_DIR}/lib/${ANDROID_ABI}/libSDL2main.a")
    add_library(SDL2main STATIC IMPORTED)
    set_target_properties(SDL2main PROPERTIES
        IMPORTED_LOCATION "${SDL2_DIR}/lib/${ANDROID_ABI}/libSDL2main.a"
    )
    set(SDL2MAIN_LIB SDL2main)
else()
    set(SDL2MAIN_LIB "")
endif()

add_library(SDL2_image SHARED IMPORTED)
set_target_properties(SDL2_image PROPERTIES
    IMPORTED_LOCATION "${SDL2_IMAGE_DIR}/lib/${ANDROID_ABI}/libSDL2_image.so"
)

# Build the game as a shared library
add_library(game SHARED
    ${GAME_SOURCES}
    ${ANDROID_WRAPPER_SOURCES}
)

# Link with SDL2 libraries and Android system libraries
target_link_libraries(game
    ${SDL2MAIN_LIB}
    SDL2
    SDL2_image
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
