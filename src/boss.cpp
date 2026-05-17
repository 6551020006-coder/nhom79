#include "boss.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Enemy::Enemy(Vector2 position, EnemyType type, Texture2D tex, Vector2 playerPos)
    : alive(true), escapedBottom(false), type(type), hp(0), scoreValue(0),
      position(position), velocity({0, 0}), speed(0),
      lastFireTime(0.0), fireInterval(0), width(0), height(0),
      timeAlive(0.0), hasImage(false)
{
    switch (type)
    {
    // -------------------------------------------------------
    // 🛸 DRONE — bay ngang, ban thang xuong
    // -------------------------------------------------------
    case EnemyType::DRONE:
        hp           = 1;
        scoreValue   = 100;
        speed        = 1.2f;
        fireInterval = 2.5f;
        width        = tex.width;
        height       = tex.height;
        velocity     = {0, speed};  // Bay thang xuong
        image        = tex;
        hasImage     = true;
        break;

    // -------------------------------------------------------
    // 🔮 SCOUT — zic zac ngang, moi con co phase rieng
    // -------------------------------------------------------
    case EnemyType::SCOUT:
        hp           = 1;
        scoreValue   = 200;
        speed        = 1.8f;
        fireInterval = 1.5f;
        width        = tex.width;
        height       = tex.height;
        velocity     = {0, 0};
        // Phase ngau nhien de moi con zic zac lech nhau
        timeAlive    = (float)(rand() % 628) / 100.0f;
        image        = tex;
        hasImage     = true;
        break;

    // -------------------------------------------------------
    // 🐙 OCTOPUS — boss lon, chi di trai/phai, khong roi man hinh
    // -------------------------------------------------------
    case EnemyType::OCTOPUS:
        hp           = 100;      // Tang mau len 100 cho trau
        scoreValue   = 3000;
        speed        = 2.5f;
        fireInterval = 0.8f;
        width        = tex.width;
        height       = tex.height;
        velocity     = {speed, 0};  // Chi di trai/phai
        image        = tex;
        hasImage     = true;
        break;

    default:
        break;
    }
}

Enemy::~Enemy() {}

// -------------------------------------------------------
// UPDATE
// -------------------------------------------------------
void Enemy::Update(Vector2 playerPos)
{
    if (!alive) return;
    timeAlive += GetFrameTime();

    switch (type)
    {
    // 🛸 DRONE — bay thang xuong tu tu
    case EnemyType::DRONE:
    {
        position.y += velocity.y;   // Chi di chuyen truc Y

        if (position.y > GetScreenHeight()) {
            alive = false;
            escapedBottom = true;
        }

        FireLaser(playerPos);
        break;
    }

    // 🔮 SCOUT — zic zac ngang, tut dan xuong, moi con lech phase
    case EnemyType::SCOUT:
    {
        // Tut xuong cham
        position.y += 0.8f;

        // Zic zac ngang theo sin, phase rieng nen khong dồn vao nhau
        position.x += sinf(timeAlive * 2.5f) * 3.0f;

        // Giu trong man hinh
        if (position.x < 0) position.x = 0;
        if (position.x + width > GetScreenWidth())
            position.x = (float)(GetScreenWidth() - width);

        if (position.y > GetScreenHeight()) {
            alive = false;
            escapedBottom = true;
        }

        FireLaser(playerPos);
        break;
    }

    // 🐙 OCTOPUS — chi di trai/phai, nay lai khi cham vien, khong roi man hinh
    case EnemyType::OCTOPUS:
    {
        // Tu tu bay xuong man hinh khi moi spawn
        if (position.y < 40) {
            position.y += speed * 0.5f;
        } else {
            // Di chuyen ngang
            position.x += velocity.x;

            // Nay lai khi cham bien trai hoac phai
            if (position.x <= 0) {
                position.x = 0;
                velocity.x = speed;   // Doi huong sang phai
            }
            if (position.x + width >= GetScreenWidth()) {
                position.x = (float)(GetScreenWidth() - width);
                velocity.x = -speed;  // Doi huong sang trai
            }

            // Giu Y co dinh gan dau man hinh, khong tut xuong
            if (position.y > 120) position.y = 120; // Gioi han mem mai hon
        }

        // KHONG BAO GIO cho alive = false (chi chet khi bi ha guc)

        FireLaser(playerPos);
        break;
    }

    default:
        break;
    }

}

// -------------------------------------------------------
// FIRE LASER
// -------------------------------------------------------
void Enemy::FireLaser(Vector2 playerPos)
{
    if (GetTime() - lastFireTime < fireInterval) return;

    switch (type)
    {
    case EnemyType::DRONE:
        lasers.push_back(Laser(
            {position.x + width / 2.0f - 2, position.y + height},
            4.0f,
            {255, 50, 50, 255}
        ));
        break;

    case EnemyType::SCOUT:
    {
        // Tinh huong ban ve phia player
        float dx = playerPos.x - (position.x + width / 2.0f);
        float dy = playerPos.y - (position.y + height);
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist > 0) {
            float nx = dx / dist;
            float ny = dy / dist;
            Vector2 vel = { 6.0f * nx, 6.0f * ny };
            if (vel.y < 1.0f) vel.y = 1.0f; // Dam bao laser luon di xuong
            lasers.push_back(Laser(
                {position.x + width / 2.0f - 2, position.y + height},
                vel,
                Color{255, 150, 0, 255}
            ));
        }
        break;
    }

    case EnemyType::OCTOPUS:
        lasers.push_back(Laser(
            {position.x + width / 2.0f - 2, position.y + height},
            Vector2{0.0f, 5.0f}, // Giua
            Color{180, 0, 255, 255}
        ));
        lasers.push_back(Laser(
            {position.x + 10.0f, position.y + height / 2.0f},
            Vector2{-2.0f, 5.0f}, // Cheo trai
            Color{180, 0, 255, 255}
        ));
        lasers.push_back(Laser(
            {position.x + width - 14.0f, position.y + height / 2.0f},
            Vector2{2.0f, 5.0f}, // Cheo phai
            Color{180, 0, 255, 255}
        ));
        break;

    default:
        break;
    }

    lastFireTime = GetTime();
}

// -------------------------------------------------------
// DRAW
// -------------------------------------------------------
void Enemy::Draw()
{
    if (!alive) return;

    if (hasImage)
        DrawTextureV(image, position, WHITE);
    else
        DrawRectangle((int)position.x, (int)position.y, width, height, RED);
}

// -------------------------------------------------------
// GET RECT
// -------------------------------------------------------
Rectangle Enemy::GetRect() const
{
    return { position.x, position.y, (float)width, (float)height };
}
