#include "laser.hpp"

Laser::Laser(Vector2 position, float speed, Color color)
{
    this->position = position;
    this->velocity = {0.0f, speed};
    this->color    = color;
    active         = true;
}

Laser::Laser(Vector2 position, Vector2 velocity, Color color)
{
    this->position = position;
    this->velocity = velocity;
    this->color    = color;
    active         = true;
}

void Laser::Draw() {
    if (active)
        DrawRectangle((int)position.x, (int)position.y, 4, 15, color);
}

void Laser::Update() {
    position.x += velocity.x;
    position.y += velocity.y;
    if (active) {
        if (position.y > GetScreenHeight() + 20 || position.y < -20 ||
            position.x > GetScreenWidth() + 20 || position.x < -20)
            active = false;
    }
}

Rectangle Laser::GetRect() const {
    return { position.x, position.y, 4, 15 };
}
