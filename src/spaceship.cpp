#include "spaceship.hpp"
#include <stdexcept>

Spaceship::Spaceship()
{
    try {
        // Load anh va xoa nen de lam trong suot
        Image img = LoadImage("Graphics/spaceship.png");
        if (img.data == nullptr) {
            throw std::runtime_error("Loi: Khong the load file Graphics/spaceship.png");
        }
        // Xoa cac pixel mau toi/nen (gan den) thanh trong suot
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                Color pixel = GetImageColor(img, x, y);
                // Neu pixel qua toi (nen den hoac gan den) -> lam trong suot
                if (pixel.r < 30 && pixel.g < 30 && pixel.b < 30) {
                    ImageDrawPixel(&img, x, y, BLANK);
                }
            }
        }
        image = LoadTextureFromImage(img);
        UnloadImage(img);
        position.x = (GetScreenWidth()  - image.width)  / 2.0f;
        position.y = GetScreenHeight() - image.height - 100;
        lastFireTime = 0.0;

        // Load am thanh tu file .wav (spaceship tu phat tieng laser cua no)
        laserSound = LoadSound("Sounds/laser.wav");
        if (laserSound.stream.buffer == nullptr) {
            throw std::runtime_error("Loi: Khong the load file Sounds/laser.wav");
        }
        SetSoundVolume(laserSound, 0.8f);
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "NGOAI LE (Spaceship): %s", e.what());
        throw; // Nem tiep de main.cpp bat
    }

    // Trang thai mac dinh
    hp                = 3;
    shieldActive      = false;
    rapidFireActive   = false;
    shieldTimer       = 0.0f;
    rapidFireTimer    = 0.0f;
    invulnerableTimer = 0.0f;
    for (int i = 0; i < 4; i++) inventory[i] = 0;
}

Spaceship::~Spaceship() {
    UnloadTexture(image);
    UnloadSound(laserSound);
}

void Spaceship::Draw() {
    // Ve hieu ung nhap nhay khi bat tu
    if (invulnerableTimer > 0.0f) {
        // Nhap nhay moi 0.1 giay
        int frame = (int)(invulnerableTimer * 10.0f);
        if (frame % 2 == 0)
            DrawTextureV(image, position, Color{255, 255, 255, 180});
        // Bo qua frame le (tao hieu ung nhap nhay)
    } else {
        DrawTextureV(image, position, WHITE);
    }

    // Ve khien neu dang kich hoat
    if (shieldActive) {
        float cx = position.x + image.width  / 2.0f;
        float cy = position.y + image.height / 2.0f;
        float r  = (float)(image.width > image.height ? image.width : image.height) / 2.0f + 8.0f;
        // Hieu ung khien nhap nhay theo thoi gian
        float alpha = 120.0f + sinf((float)GetTime() * 5.0f) * 80.0f;
        DrawCircleLines((int)cx, (int)cy, r,      Color{80, 180, 255, (unsigned char)alpha});
        DrawCircleLines((int)cx, (int)cy, r + 3,  Color{80, 180, 255, (unsigned char)(alpha / 2)});
    }
}

void Spaceship::Update() {
    float dt = GetFrameTime();

    if (shieldActive) {
        shieldTimer -= dt;
        if (shieldTimer <= 0.0f) { shieldActive = false; shieldTimer = 0.0f; }
    }
    if (rapidFireActive) {
        rapidFireTimer -= dt;
        if (rapidFireTimer <= 0.0f) { rapidFireActive = false; rapidFireTimer = 0.0f; }
    }
    if (invulnerableTimer > 0.0f) {
        invulnerableTimer -= dt;
        if (invulnerableTimer < 0.0f) invulnerableTimer = 0.0f;
    }
}

void Spaceship::MoveLeft() {
    position.x -= 20;
    if (position.x < 25) position.x = 25;
}

void Spaceship::MoveRight() {
    position.x += 20;
    if (position.x > GetScreenWidth() - image.width - 25)
        position.x = GetScreenWidth() - image.width - 25;
}

void Spaceship::MoveUp() {
    position.y -= 15;
    if (position.y < GetScreenHeight() / 2)  // Chi cho di len nua man hinh duoi
        position.y = GetScreenHeight() / 2;
}

void Spaceship::MoveDown() {
    position.y += 15;
    if (position.y > GetScreenHeight() - image.height - 10)
        position.y = GetScreenHeight() - image.height - 10;
}

void Spaceship::FireLaser()
{
    float interval = rapidFireActive ? 0.1f : 0.35f;

    if (GetTime() - lastFireTime >= interval) {
        if (rapidFireActive) {
            lasers.push_back(Laser({position.x + image.width / 2.0f - 2, position.y}, -12.0f));
            lasers.push_back(Laser({position.x + image.width / 2.0f - 12, position.y + 10}, -12.0f, Color{255, 200, 80, 255}));
            lasers.push_back(Laser({position.x + image.width / 2.0f + 8,  position.y + 10}, -12.0f, Color{255, 200, 80, 255}));
        } else {
            lasers.push_back(Laser({position.x + image.width / 2.0f - 2, position.y}, -9.0f));
        }
        
        PlaySound(laserSound);
        lastFireTime = GetTime();
    }
}
void Spaceship::Reset()
{
    position.x = (GetScreenWidth()  - image.width)  / 2.0f;
    position.y = GetScreenHeight() - image.height - 100;
    lasers.clear();

    hp                = 3;
    shieldActive      = false;
    rapidFireActive   = false;
    shieldTimer       = 0.0f;
    rapidFireTimer    = 0.0f;
    invulnerableTimer = 0.0f;
    for (int i = 0; i < 4; i++) inventory[i] = 0;
}

// --- AP DUNG KY NANG ---

void Spaceship::ApplyHeal() {
    hp++;
    if (hp > 5) hp = 5;  // Gioi han toi da 5 mang
}

void Spaceship::ApplyShield() {
    shieldActive = true;
    shieldTimer  = 5.0f;  // 5 giay
}

void Spaceship::ApplyRapidFire() {
    rapidFireActive = true;
    rapidFireTimer  = 6.0f;  // 6 giay
}
