#pragma once
#include <raylib.h>

class Laser {
public:
    Laser(Vector2 position, float speed, Color color = {243, 216, 63, 255});
    Laser(Vector2 position, Vector2 velocity, Color color = {243, 216, 63, 255});
    void Draw();
    void Update();
    Rectangle GetRect() const;  // Them de kiem tra collision
    bool active;

private:
    Vector2 position;
    Vector2 velocity;
    Color color;
};
