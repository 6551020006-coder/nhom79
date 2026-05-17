#pragma once
#include <raylib.h>
#include <cmath>
#include "laser.hpp"
#include <vector>

class Spaceship {
public:
    Spaceship();
    ~Spaceship();

    void Draw();
    void Update();
    void Reset();
    void MoveLeft();
    void MoveRight();
    void MoveUp();
    void MoveDown();
    void FireLaser();

    void ApplyHeal();
    void ApplyShield();
    void ApplyRapidFire();

    Vector2 GetPosition() const { return position; }
    int GetWidth()  const { return image.width;  }
    int GetHeight() const { return image.height; }

    std::vector<Laser> lasers;

    int   hp;
    bool  shieldActive;
    bool  rapidFireActive;
    float shieldTimer;
    float rapidFireTimer;
    float invulnerableTimer;

    int inventory[4];

private:
    Texture2D image;
    Vector2   position;
    double    lastFireTime;
    Sound     laserSound;   // <-- them vao day
};
