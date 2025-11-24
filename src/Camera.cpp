#include "Camera.hpp"
#include <algorithm>

Camera::Camera() 
    : position(0, 0), width(768), height(576), worldWidth(50 * 96), worldHeight(50 * 96) {
}

Camera::Camera(int w, int h) 
    : position(0, 0), width(w), height(h), worldWidth(50 * 96), worldHeight(50 * 96) {
}

Camera::~Camera() {
}

void Camera::update(Vector2D playerPosition) {
    // Κεντράρει την κάμερα στον παίκτη
    position.x = playerPosition.x - (width / 2.0f);
    position.y = playerPosition.y - (height / 2.0f);
    
    // Περιορίζει την κάμερα στα όρια του κόσμου
    if (position.x < 0) position.x = 0;
    if (position.y < 0) position.y = 0;
    if (position.x + width > worldWidth) position.x = worldWidth - width;
    if (position.y + height > worldHeight) position.y = worldHeight - height;
}

Vector2D Camera::getPosition() const {
    return position;
}

int Camera::worldToScreenX(float worldX) const {
    return static_cast<int>(worldX - position.x);
}

int Camera::worldToScreenY(float worldY) const {
    return static_cast<int>(worldY - position.y);
}
