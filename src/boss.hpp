#pragma once
#include <raylib.h>
#include "laser.hpp"
#include <vector>

enum class EnemyType {
    DRONE,      // Bay ngang, ban thang xuong
    SCOUT,      // Duoi theo player, ban nhanh
    OCTOPUS     // Bay hinh sin, ban da huong, 1 con to
};

class Enemy {
public:
    Enemy(Vector2 position, EnemyType type, Texture2D tex, Vector2 playerPos = {0, 0});
    ~Enemy();
    void Draw();
    void Update(Vector2 playerPos);
    void FireLaser(Vector2 playerPos);
    Rectangle GetRect() const;

    bool alive;
    bool escapedBottom;  // Thoat ra ngoai man hinh phia duoi
    EnemyType type;
    int hp;
    int scoreValue;
    std::vector<Laser> lasers;

private:
    Vector2 position;
    Vector2 velocity;
    float speed;
    double lastFireTime;
    float fireInterval;
    int width, height;
    float timeAlive;
    Texture2D image;
    bool hasImage;
};
