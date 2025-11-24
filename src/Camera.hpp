#ifndef Camera_hpp
#define Camera_hpp

#include "Vector2D.hpp"

class Camera {
public:
    Camera();
    Camera(int width, int height);
    ~Camera();

    void update(Vector2D playerPosition);
    
    // Επιστρέφει τη θέση της κάμερας (πάνω-αριστερή γωνία του view)
    Vector2D getPosition() const;
    
    // Μετατρέπει συντεταγμένες κόσμου σε συντεταγμένες οθόνης
    int worldToScreenX(float worldX) const;
    int worldToScreenY(float worldY) const;
    
    // Επιστρέφει τα όρια της κάμερας
    int getX() const { return static_cast<int>(position.x); }
    int getY() const { return static_cast<int>(position.y); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    Vector2D position;  // Camera position in world coordinates (top-left corner)
    int width;          // Viewport width
    int height;         // Viewport height
    int worldWidth;     // Total world width
    int worldHeight;    // Total world height
};

#endif
