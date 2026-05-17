#include "game1.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

static float RandomX(int enemyWidth) {
  return (float)(rand() % (GetScreenWidth() - enemyWidth));
}

// Ve trai tim bang shapes
static void DrawHeartShape(float cx, float cy, float size, Color color) {
  float r = size * 0.27f;
  DrawCircleV({cx - r * 0.72f, cy - size * 0.08f}, r, color);
  DrawCircleV({cx + r * 0.72f, cy - size * 0.08f}, r, color);
  Vector2 v1 = {cx - size * 0.50f, cy - size * 0.04f};
  Vector2 v2 = {cx + size * 0.50f, cy - size * 0.04f};
  Vector2 v3 = {cx, cy + size * 0.50f};
  DrawTriangle(v2, v1, v3, color);
}

// Ve nen cuon: cat mot "slice" dung chieu cao man hinh tu texture va keo rong ra
static void DrawScrollingBg(Texture2D &tex, float scrollY) {
  float sW = (float)GetScreenWidth();
  float sH = (float)GetScreenHeight();
  float tW = (float)tex.width;
  float tH = (float)tex.height;

  // srcY: vi tri bat dau trong texture (theo chieu doc cua texture)
  float srcY = fmodf(scrollY, tH);
  if (srcY < 0) srcY += tH;

  float remaining = tH - srcY; // so pixel con lai den day texture

  if (remaining >= sH) {
    // Toan bo man hinh nam trong mot slice cua texture
    DrawTexturePro(tex, {0, srcY, tW, sH}, {0, 0, sW, sH}, {0,0}, 0, WHITE);
  } else {
    // Phan 1: tu srcY den cuoi texture — ve o tren man hinh
    float destH1 = remaining / sH * sH; // chieu cao hien thi phan 1
    DrawTexturePro(tex, {0, srcY, tW, remaining}, {0, 0, sW, destH1}, {0,0}, 0, WHITE);
    // Phan 2: tu dau texture — ve phan duoi man hinh
    float srcH2 = sH - remaining;
    DrawTexturePro(tex, {0, 0, tW, srcH2}, {0, destH1, sW, sH - destH1}, {0,0}, 0, WHITE);
  }
}

// -------------------------------------------------------
// AM THANH
// -------------------------------------------------------
void Game::PlayBGM() {
  if (bgmPlaying)
    return;
  bgmPlaying = true;
  PlayMusicStream(bgm[currentBgm]);
  SetMusicVolume(bgm[currentBgm], 0.5f);
}

void Game::StopBGM() {
  if (!bgmPlaying)
    return;
  bgmPlaying = false;
  StopMusicStream(bgm[currentBgm]);
}

void Game::UpdateBGM() {
  if (!bgmPlaying)
    return;
  UpdateMusicStream(bgm[currentBgm]);
  // Chuyen bai ke tiep khi het bai hien tai
  if (GetMusicTimePlayed(bgm[currentBgm]) >=
      GetMusicTimeLength(bgm[currentBgm]) - 0.05f) {
    StopMusicStream(bgm[currentBgm]);
    currentBgm = (currentBgm + 1) % 3;
    PlayMusicStream(bgm[currentBgm]);
    SetMusicVolume(bgm[currentBgm], 0.5f);
  }
}

// -------------------------------------------------------
// CONSTRUCTOR / DESTRUCTOR
// -------------------------------------------------------
Game::Game() {
  try {
    droneTexture = LoadTexture("Graphics/drone.png");
    scoutTexture = LoadTexture("Graphics/scout.png");
    octopusTexture = LoadTexture("Graphics/octopus.png");
    bgTexture = LoadTexture("Graphics/galaxy_seamless.png");
    if (droneTexture.id == 0 || scoutTexture.id == 0 || octopusTexture.id == 0 || bgTexture.id == 0) {
        throw std::runtime_error("Loi: Khong the load mot hoac nhieu file hinh anh (drone, scout, octopus, bg) trong Graphics/");
    }
    SetTextureFilter(bgTexture, TEXTURE_FILTER_BILINEAR);

    texShield = LoadTexture("Graphics/shield.png");
    texRapidFire = LoadTexture("Graphics/rapidfire.png");
    texHeal = LoadTexture("Graphics/heal.png");
    texBomb = LoadTexture("Graphics/bomb.png");
    if (texShield.id == 0 || texRapidFire.id == 0 || texHeal.id == 0 || texBomb.id == 0) {
        throw std::runtime_error("Loi: Khong the load hinh anh Powerup trong Graphics/");
    }

    // Load am thanh tu file .mp3 / .wav
    sfxLaser = LoadSound("Sounds/laser.wav");
    sfxExplode = LoadSound("Sounds/explosion.wav");
    sfxEnemyLaser = LoadSound("Sounds/enemy_laser.wav");
    sfxHit = LoadSound("Sounds/hit.wav");
    sfxPowerup = LoadSound("Sounds/powerup.wav");
    sfxHurt = LoadSound("Sounds/hurt.wav");
    sfxBoss = LoadSound("Sounds/boss.wav");
    sfxWin = LoadSound("Sounds/win.wav");

    if (sfxLaser.stream.buffer == nullptr || sfxExplode.stream.buffer == nullptr || sfxWin.stream.buffer == nullptr) {
        throw std::runtime_error("Loi: Khong the load mot hoac nhieu file am thanh (laser, explosion...) trong Sounds/");
    }

    SetSoundVolume(sfxLaser, 1.0f);
    SetSoundVolume(sfxExplode, 1.0f);
    SetSoundVolume(sfxEnemyLaser, 1.0f);
    SetSoundVolume(sfxHit, 1.0f);
    SetSoundVolume(sfxPowerup, 1.0f);
    SetSoundVolume(sfxHurt, 1.0f);
    SetSoundVolume(sfxBoss, 1.0f);
    SetSoundVolume(sfxWin, 1.0f);

    // Load nhac nen
    bgm[0] = LoadMusicStream("Sounds/bgm1.mp3");
    bgm[1] = LoadMusicStream("Sounds/bgm2.mp3");
    bgm[2] = LoadMusicStream("Sounds/bgm3.mp3");
    for (int i = 0; i < 3; i++) {
      if (bgm[i].stream.buffer == nullptr) {
          throw std::runtime_error("Loi: Khong the load file nhac nen Sounds/bgm" + std::to_string(i+1) + ".mp3");
      }
      bgm[i].looping = false;
      SetMusicVolume(bgm[i], 0.5f);
    }
  } catch (const std::exception& e) {
      TraceLog(LOG_ERROR, "NGOAI LE (Game Init): %s", e.what());
      throw; // Nem tiep de main.cpp bat và ket thuc chuong trinh
  }
  currentBgm = 0;
  bgmPlaying = false;

  bgY = 0.0f;
  score = 0;
  LoadHighScore();

  state = GameState::MENU;
  menuPulse = 0.0f;
  menuBgY = 0.0f;
  gameOverPulse = 0.0f;
  victoryPulse = 0.0f;
  godMode = false;
  isGameActive = false;
  currentMenuOption = 0;
  cameraShakeTimer = 0.0f;

  powerupsDropped = 0;
  maxPowerupsPerWave = 0;
  lastPowerupDropTime = 0.0;
  nextPowerupInterval = 0.0f;

  StartWave(1);
  PlayBGM();
}

Game::~Game() {
  UnloadTexture(droneTexture);
  UnloadTexture(scoutTexture);
  UnloadTexture(octopusTexture);
  UnloadTexture(bgTexture);
  UnloadTexture(texShield);
  UnloadTexture(texRapidFire);
  UnloadTexture(texHeal);
  UnloadTexture(texBomb);

  UnloadSound(sfxLaser);
  UnloadSound(sfxExplode);
  UnloadSound(sfxEnemyLaser);
  UnloadSound(sfxHit);
  UnloadSound(sfxPowerup);
  UnloadSound(sfxHurt);
  UnloadSound(sfxBoss);
  UnloadSound(sfxWin);

  for (int i = 0; i < 3; i++)
    UnloadMusicStream(bgm[i]);
}

void Game::LoadHighScore() {
  try {
    std::ifstream file("highscore.txt");
    if (!file.is_open()) {
      throw std::runtime_error("Khong the mo file highscore.txt de doc diem cao");
    }
    file >> highScore;
    file.close();
  } catch (const std::exception& e) {
    TraceLog(LOG_WARNING, "NGOAI LE (LoadHighScore): %s", e.what());
    highScore = 0;
  }
}

void Game::SaveHighScore() {
  try {
    std::ofstream file("highscore.txt");
    if (!file.is_open()) {
      throw std::runtime_error("Khong the mo file highscore.txt de ghi diem cao");
    }
    file << highScore;
    file.close();
  } catch (const std::exception& e) {
    TraceLog(LOG_WARNING, "NGOAI LE (SaveHighScore): %s", e.what());
  }
}

// -------------------------------------------------------
// WAVE
// -------------------------------------------------------
void Game::StartWave(int wave) {
  currentWave = wave;
  spawnedCount = 0;
  lastSpawnTime = GetTime();
  waveCleared = false;
  octopusSpawned = false;
  enemies.clear();
  powerups.clear();

  powerupsDropped = 0;
  maxPowerupsPerWave = 4 + rand() % 2;
  lastPowerupDropTime = GetTime();
  nextPowerupInterval = 8.0f + (float)(rand() % 8);

  if (wave == 1) {
    spawnQueue = 30;
    spawnInterval = 1.8f;
    maxOnScreen = 6;
  } else if (wave == 2) {
    spawnQueue = 25;
    spawnInterval = 1.5f;
    maxOnScreen = 7;
  }
}

void Game::UpdateSpawner() {
  if (spawnedCount >= spawnQueue)
    return;
  if ((int)enemies.size() >= maxOnScreen)
    return;
  if (GetTime() - lastSpawnTime < spawnInterval)
    return;

  int roll = rand() % 100;
  int scoutChance = (currentWave == 1) ? 30 : 40;

  if (roll < scoutChance)
    enemies.push_back(
        Enemy({RandomX(64), -70}, EnemyType::SCOUT, scoutTexture));
  else
    enemies.push_back(
        Enemy({RandomX(64), -70}, EnemyType::DRONE, droneTexture));

  spawnedCount++;
  lastSpawnTime = GetTime();
}

// -------------------------------------------------------
// POWERUP ROI TU TREN TROI
// -------------------------------------------------------
void Game::UpdatePowerupSpawner() {
  if (state != GameState::PLAYING)
    return;
  if (powerupsDropped >= maxPowerupsPerWave)
    return;
  if (GetTime() - lastPowerupDropTime < nextPowerupInterval)
    return;

  float x = 32.0f + (float)(rand() % (GetScreenWidth() - 96));
  Vector2 pos = {x, -60.0f};

  int roll = rand() % 4;
  switch (roll) {
  case 0:
    powerups.push_back(Powerup(pos, PowerupType::HEAL, texHeal));
    break;
  case 1:
    powerups.push_back(Powerup(pos, PowerupType::RAPID_FIRE, texRapidFire));
    break;
  case 2:
    powerups.push_back(Powerup(pos, PowerupType::SHIELD, texShield));
    break;
  case 3:
    powerups.push_back(Powerup(pos, PowerupType::BOMB, texBomb));
    break;
  }

  powerupsDropped++;
  lastPowerupDropTime = GetTime();
  nextPowerupInterval = 8.0f + (float)(rand() % 8);
}

// -------------------------------------------------------
// COLLISION — dan spaceship ban enemy
// -------------------------------------------------------
void Game::CheckCollisions() {
  for (auto &laser : spaceship.lasers) {
    if (!laser.active)
      continue;

    for (auto &enemy : enemies) {
      if (!enemy.alive || enemy.GetRect().y < 0)
        continue;

      if (CheckCollisionRecs(laser.GetRect(), enemy.GetRect())) {
        laser.active = false;
        enemy.hp -= 1;

        if (enemy.hp <= 0) {
          enemy.alive = false;
          score += enemy.scoreValue;
          StopSound(sfxExplode);
          PlaySound(sfxExplode);
          SpawnExplosion({enemy.GetRect().x + enemy.GetRect().width / 2,
                          enemy.GetRect().y + enemy.GetRect().height / 2},
                         ORANGE);
        } else {
          StopSound(sfxHit);
          PlaySound(sfxHit);
        }
        break;
      }
    }
  }
}

// -------------------------------------------------------
// COLLISION — dan dich ban trung spaceship
// -------------------------------------------------------
void Game::CheckEnemyLaserCollisions() {
  if (spaceship.shieldActive)
    return;
  if (godMode)
    return;
  if (spaceship.invulnerableTimer > 0.0f)
    return;

  Rectangle playerRect = {spaceship.GetPosition().x, spaceship.GetPosition().y,
                          (float)spaceship.GetWidth(),
                          (float)spaceship.GetHeight()};

  for (auto &laser : enemyLasers) {
    if (!laser.active)
      continue;
    if (CheckCollisionRecs(laser.GetRect(), playerRect)) {
      laser.active = false;
      spaceship.hp--;
      spaceship.invulnerableTimer = 1.0f;
      StopSound(sfxHurt);
      PlaySound(sfxHurt);
      cameraShakeTimer = 0.3f;
      return;
    }
  }

  for (auto &enemy : enemies) {
    if (!enemy.alive)
      continue;
    if (CheckCollisionRecs(enemy.GetRect(), playerRect)) {
      if (enemy.type != EnemyType::OCTOPUS) {
        enemy.alive = false;
        StopSound(sfxExplode);
        PlaySound(sfxExplode);
        SpawnExplosion({enemy.GetRect().x + enemy.GetRect().width / 2,
                        enemy.GetRect().y + enemy.GetRect().height / 2},
                       ORANGE);
      }
      spaceship.hp--;
      spaceship.invulnerableTimer = 1.0f;
      StopSound(sfxHurt);
      PlaySound(sfxHurt);
      cameraShakeTimer = 0.3f;
      spaceship.MoveDown();
      return;
    }
  }
}

// -------------------------------------------------------
// COLLISION — spaceship cham powerup
// -------------------------------------------------------
void Game::CheckPowerupCollisions() {
  Rectangle playerRect = {spaceship.GetPosition().x, spaceship.GetPosition().y,
                          (float)spaceship.GetWidth(),
                          (float)spaceship.GetHeight()};

  for (auto &p : powerups) {
    if (!p.active)
      continue;
    if (!CheckCollisionRecs(playerRect, p.GetRect()))
      continue;
    p.active = false;
    StopSound(sfxPowerup);
    PlaySound(sfxPowerup);
    spaceship.inventory[(int)p.type]++;
    TraceLog(LOG_INFO, TextFormat("POWERUP PICKED: %d", (int)p.type));
  }
}

// -------------------------------------------------------
// HEALTH BAR enemy
// -------------------------------------------------------
void Game::DrawHealthBar(const Enemy &enemy) {
  if (!enemy.alive)
    return;
  Rectangle rect = enemy.GetRect();

  int maxHp = 1;
  if (enemy.type == EnemyType::SCOUT)
    maxHp = 1;
  if (enemy.type == EnemyType::OCTOPUS)
    maxHp = 100;

  int barW = (int)rect.width;
  int barH = 4;
  if (enemy.type == EnemyType::OCTOPUS)
    barW = 60;

  int barX = (int)rect.x + (int)((rect.width - barW) / 2);
  int barY = (int)rect.y - 10;

  DrawRectangle(barX, barY, barW, barH, {80, 80, 80, 180});
  float ratio = (float)enemy.hp / (float)maxHp;
  Color col = {(unsigned char)(255 * (1.0f - ratio)),
               (unsigned char)(255 * ratio), 0, 255};
  DrawRectangle(barX, barY, (int)(barW * ratio), barH, col);
}

// -------------------------------------------------------
// HUD
// -------------------------------------------------------
void Game::DrawHUD() {
  DrawText(TextFormat("Man: %d", currentWave), 10, 10, 20, WHITE);
  DrawText(TextFormat("Score: %d", score), 10, 35, 20, WHITE);
  DrawText(TextFormat("High Score: %d", highScore), 10, 58, 18, RED);
  int remaining = spawnQueue - spawnedCount;
  DrawText(TextFormat("Enemies: %d", remaining), 10, 80, 20, WHITE);

  int maxHeartsDisplay = 5;
  int heartSize = 24;
  int heartY = GetScreenHeight() - 34;
  for (int i = 0; i < maxHeartsDisplay; i++) {
    int hx = 10 + i * (heartSize + 4);
    float cx = hx + heartSize * 0.5f;
    float cy = heartY + heartSize * 0.5f;
    if (i < spaceship.hp) {
      float pulse = 0.85f + sinf((float)GetTime() * 4.0f + i) * 0.15f;
      unsigned char r = (unsigned char)(255 * pulse);
      unsigned char g = (unsigned char)(50 * pulse);
      DrawHeartShape(cx, cy, (float)heartSize, Color{r, g, 80, 255});
    } else if (i < 3) {
      DrawHeartShape(cx, cy, (float)heartSize, Color{80, 30, 30, 140});
    }
  }

  if (godMode) {
    float gp = (float)GetTime() * 4.0f;
    unsigned char ga = (unsigned char)(180 + sinf(gp) * 75);
    unsigned char gy = (unsigned char)(200 + sinf(gp * 1.3f) * 55);
    const char *godText = "\xe2\x9a\xa1 GOD MODE";
    int gw = MeasureText(godText, 18);
    DrawRectangleRounded(
        {(float)(GetScreenWidth() / 2 - gw / 2 - 10), 84, (float)(gw + 20), 26}, 0.5f, 6,
        Color{60, 40, 0, (unsigned char)(ga * 0.5f)});
    DrawRectangleLinesEx(
        {(float)(GetScreenWidth() / 2 - gw / 2 - 10), 84, (float)(gw + 20), 26}, 1,
        Color{255, gy, 0, ga});
    DrawText(godText, GetScreenWidth() / 2 - gw / 2, 88, 18, Color{255, gy, 30, ga});
  }

  int iconSize = 40;
  int marginR = 10;
  int iconY = GetScreenHeight() - iconSize - 10;

  Texture2D texs[4] = {texHeal, texRapidFire, texShield, texBomb};
  const char *keys[4] = {"Q", "W", "E", "R"};
  Color activeColors[4] = {GREEN, YELLOW, SKYBLUE, RED};

  for (int i = 0; i < 4; i++) {
    int ix = GetScreenWidth() - marginR - (4 - i) * (iconSize + 8);
    int count = spaceship.inventory[i];
    Color tint = (count > 0) ? WHITE : Color{255, 255, 255, 80};
    Color borderColor = (count > 0) ? activeColors[i] : GRAY;

    DrawTexturePro(texs[i], {0, 0, (float)texs[i].width, (float)texs[i].height},
                   {(float)ix, (float)iconY, (float)iconSize, (float)iconSize},
                   {0, 0}, 0, tint);
    DrawRectangleLinesEx(
        {(float)ix, (float)iconY, (float)iconSize, (float)iconSize}, 1,
        borderColor);
    DrawText(keys[i], ix + 4, iconY - 16, 14, borderColor);
    if (count > 0)
      DrawText(TextFormat("%d", count), ix + iconSize - 12,
               iconY + iconSize - 16, 16, WHITE);
  }

  if (spaceship.rapidFireActive) {
    int ix = GetScreenWidth() - marginR - 3 * (iconSize + 8);
    DrawText(TextFormat("%.0fs", spaceship.rapidFireTimer), ix + 20, iconY - 16,
             14, YELLOW);
  }
  if (spaceship.shieldActive) {
    int ix = GetScreenWidth() - marginR - 2 * (iconSize + 8);
    DrawText(TextFormat("%.0fs", spaceship.shieldTimer), ix + 20, iconY - 16,
             14, SKYBLUE);
  }

  Rectangle pauseBtn = {(float)GetScreenWidth() - 50, 10, 40, 40};
  bool isHover = CheckCollisionPointRec(GetMousePosition(), pauseBtn);
  Color btnColor =
      isHover ? Color{255, 255, 255, 150} : Color{255, 255, 255, 80};
  DrawRectangleRounded(pauseBtn, 0.3f, 4, btnColor);
  DrawRectangleLinesEx(pauseBtn, 2, WHITE);
  DrawRectangle((int)pauseBtn.x + 12, (int)pauseBtn.y + 10, 6, 20, WHITE);
  DrawRectangle((int)pauseBtn.x + 22, (int)pauseBtn.y + 10, 6, 20, WHITE);
}

// -------------------------------------------------------
// DRAW PAUSED
// -------------------------------------------------------
void Game::DrawPaused() {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 150});

  int cy = GetScreenHeight() / 2;

  const char *title = "TAM DUNG";
  int titleSize = 64;
  int titleW = MeasureText(title, titleSize);
  DrawText(title, GetScreenWidth() / 2 - titleW / 2 + 3, cy - 100 + 3, titleSize,
           Color{0, 0, 0, 180});
  DrawText(title, GetScreenWidth() / 2 - titleW / 2, cy - 100, titleSize,
           Color{80, 200, 255, 255});

  const char *resumeTxt = ">> P hoac ESC de TIEP TUC <<";
  int rW = MeasureText(resumeTxt, 20);
  DrawText(resumeTxt, GetScreenWidth() / 2 - rW / 2, cy + 10, 20, WHITE);

  const char *quitTxt = ">> ENTER de ve MENU chinh <<";
  int qW = MeasureText(quitTxt, 20);
  DrawText(quitTxt, GetScreenWidth() / 2 - qW / 2, cy + 60, 20, Color{255, 100, 100, 255});
}

// -------------------------------------------------------
// DRAW MENU
// -------------------------------------------------------
void Game::DrawMenu() {
  DrawScrollingBg(bgTexture, menuBgY);
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 150});

  int cy = GetScreenHeight() / 2;

  const char *title = "TAU VU TRU";
  int titleSize = 72;
  int titleW = MeasureText(title, titleSize);
  DrawText(title, GetScreenWidth() / 2 - titleW / 2 + 3, cy - 200 + 3, titleSize,
           Color{0, 0, 0, 180});
  DrawText(title, GetScreenWidth() / 2 - titleW / 2, cy - 200, titleSize,
           Color{80, 200, 255, 255});
  DrawRectangle(GetScreenWidth() / 2 - 160, cy - 110, 320, 2, Color{80, 200, 255, 160});

  const char *highScoreText = TextFormat("DIEM CAO NHAT: %d", highScore);
  int hsW = MeasureText(highScoreText, 20);
  DrawText(highScoreText, GetScreenWidth() / 2 - hsW / 2, cy - 80, 20, Color{255, 215, 0, 255});

  const char *options[3] = {"CHOI MOI", "CHOI TIEP", "HUONG DAN CHOI"};
  int startY = cy - 30;
  for (int i = 0; i < 3; i++) {
    Color textColor = WHITE;
    if (i == 1 && !isGameActive)
      textColor = Color{100, 100, 100, 255};

    int fontSize = 28;
    int txtW = MeasureText(options[i], fontSize);
    int txtX = GetScreenWidth() / 2 - txtW / 2;
    int txtY = startY + i * 50;

    if (currentMenuOption == i) {
      float alpha = 150.0f + sinf(menuPulse * 2.0f) * 100.0f;
      DrawRectangle(txtX - 20, txtY - 5, txtW + 40, fontSize + 10,
                    Color{80, 180, 255, (unsigned char)alpha});
      textColor = BLACK;
    }
    DrawText(options[i], txtX, txtY, fontSize, textColor);
  }

  const char *hint = "Dung UP/DOWN de chon, ENTER de xac nhan";
  int hintW = MeasureText(hint, 16);
  DrawText(hint, GetScreenWidth() / 2 - hintW / 2, cy + 200, 16, Color{160, 160, 160, 180});
}

// -------------------------------------------------------
// DRAW INSTRUCTIONS
// -------------------------------------------------------
void Game::DrawInstructions() {
  DrawScrollingBg(bgTexture, menuBgY);
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

  int cy = GetScreenHeight() / 2;

  const char *title = "HUONG DAN CHOI";
  int titleW = MeasureText(title, 40);
  DrawText(title, GetScreenWidth() / 2 - titleW / 2, cy - 300, 40, Color{80, 200, 255, 255});

  int y = cy - 200;
  DrawText("- Di chuyen: Phim mui ten (Len, Xuong, Trai, Phai)", 100, y, 20,
           WHITE);
  y += 40;
  DrawText("- Ban laser: Phim SPACE", 100, y, 20, WHITE);
  y += 40;
  DrawText("- Tu dong nhat vat pham vao tui do (goc phai duoi).", 100, y, 20,
           YELLOW);
  y += 40;
  DrawText("- Su dung ky nang:", 100, y, 20, WHITE);
  y += 30;
  DrawText("    + Phim Q: Hoi 1 mau", 100, y, 20, GREEN);
  y += 30;
  DrawText("    + Phim W: Ban nhanh (3 tia)", 100, y, 20, ORANGE);
  y += 30;
  DrawText("    + Phim E: Bat khien", 100, y, 20, SKYBLUE);
  y += 30;
  DrawText("    + Phim R: Dat bom (Tieu diet toan bo man hinh)", 100, y, 20,
           RED);
  y += 40;
  DrawText("- Phim M: Bat/Tat nhac", 100, y, 20, LIGHTGRAY);
  y += 30;
  DrawText("- Phim Shift + G: Bat tu (God Mode)", 100, y, 20, LIGHTGRAY);
  y += 30;
  DrawText("- Phim P hoac ESC hoac Click nut Pause de TAM DUNG", 100, y, 20,
           LIGHTGRAY);
  y += 50;

  const char *exitTxt = ">> Nhan ESC hoac ENTER de quay lai <<";
  int exitW = MeasureText(exitTxt, 20);
  DrawText(exitTxt, GetScreenWidth() / 2 - exitW / 2, cy + 250, 20, Color{80, 255, 80, 255});
}

// -------------------------------------------------------
// DRAW GAME OVER
// -------------------------------------------------------
void Game::DrawGameOver() {
  DrawScrollingBg(bgTexture, menuBgY);
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 170});

  int cy = GetScreenHeight() / 2;

  int heartY = cy - 170, heartSize = 52, heartSpacing = 70;
  int totalW = 3 * heartSize + 2 * (heartSpacing - heartSize);
  int startX = (GetScreenWidth() - totalW) / 2 - heartSize / 2;
  for (int i = 0; i < 3; i++) {
    int hx = startX + i * heartSpacing;
    float flicker = 0.4f + sinf(gameOverPulse * 2.0f + i) * 0.15f;
    unsigned char alpha = (unsigned char)(flicker * 160);
    DrawHeartShape(hx + heartSize * 0.5f, heartY + heartSize * 0.5f,
                   (float)heartSize, Color{180, 20, 20, alpha});
  }

  const char *goText = "GAME OVER";
  int goSize = 72;
  int goW = MeasureText(goText, goSize);
  DrawText(goText, GetScreenWidth() / 2 - goW / 2 + 3, cy - 100 + 3, goSize, Color{0, 0, 0, 180});
  float pulse = 0.85f + sinf(gameOverPulse * 2.5f) * 0.15f;
  DrawText(goText, GetScreenWidth() / 2 - goW / 2, cy - 100, goSize,
           Color{(unsigned char)(255 * pulse), (unsigned char)(60 * pulse), 50,
                 255});

  const char *scoreText = TextFormat("Diem so: %d", score);
  int sW = MeasureText(scoreText, 28);
  DrawText(scoreText, GetScreenWidth() / 2 - sW / 2, cy - 10, 28, Color{255, 220, 80, 230});
  DrawRectangle(GetScreenWidth() / 2 - 160, cy + 34, 320, 2, Color{200, 60, 60, 160});

  float alpha2 = 180.0f + sinf(gameOverPulse) * 70.0f;
  float scale2 = 1.0f + sinf(gameOverPulse) * 0.04f;
  const char *btnText = ">> ESC hoac ENTER de ve MENU <<";
  int btnSize = (int)(24 * scale2);
  int btnW = MeasureText(btnText, btnSize);
  int bx = GetScreenWidth() / 2 - btnW / 2 - 18, by = cy + 60, bw = btnW + 36, bh = btnSize + 20;
  DrawRectangleRounded({(float)bx, (float)by, (float)bw, (float)bh}, 0.4f, 8,
                       Color{180, 30, 30, (unsigned char)(alpha2 * 0.35f)});
  DrawRectangleLinesEx({(float)bx, (float)by, (float)bw, (float)bh}, 2,
                       Color{255, 80, 80, (unsigned char)alpha2});
  DrawText(btnText, GetScreenWidth() / 2 - btnW / 2, by + 10, btnSize,
           Color{255, 255, 255, (unsigned char)alpha2});

  const char *quit = "ESC: Thoat";
  int qW = MeasureText(quit, 16);
  DrawText(quit, GetScreenWidth() / 2 - qW / 2, cy + 174, 16, Color{160, 160, 160, 180});
}

// -------------------------------------------------------
// DRAW VICTORY
// -------------------------------------------------------
void Game::DrawVictory() {
  DrawScrollingBg(bgTexture, menuBgY);
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 140});

  int screenCenterY = GetScreenHeight() / 2;

  for (int i = 0; i < 30; i++) {
    float t = victoryPulse + i * 0.7f;
    float px = 100.0f + (float)(i * 61 % (int)(GetScreenWidth() - 200));
    float py = 60.0f + (float)(i * 37 % 200);
    float sz = 2.0f + sinf(t * 3.0f + i) * 2.0f;
    float brightness = 0.5f + sinf(t * 4.0f + i * 1.3f) * 0.5f;
    Color starColors[4] = {
        {255, 220, 60, (unsigned char)(200 * brightness)},
        {255, 140, 20, (unsigned char)(200 * brightness)},
        {80, 200, 255, (unsigned char)(200 * brightness)},
        {255, 100, 200, (unsigned char)(200 * brightness)},
    };
    DrawCircle((int)px, (int)py, sz, starColors[i % 4]);
  }

  float ringAlpha = 130.0f + sinf(victoryPulse * 2.0f) * 60.0f;
  DrawCircleLines(GetScreenWidth() / 2, screenCenterY - 130, 90.0f + sinf(victoryPulse) * 5.0f,
                  Color{255, 200, 50, (unsigned char)ringAlpha});
  DrawCircleLines(GetScreenWidth() / 2, screenCenterY - 130, 82.0f + sinf(victoryPulse) * 5.0f,
                  Color{255, 200, 50, (unsigned char)(ringAlpha * 0.5f)});

  {
    // Ve hinh cup chien thang
    float tp = 1.0f + sinf(victoryPulse * 3.0f) * 0.12f;
    unsigned char tg = (unsigned char)(210 * tp);
    unsigned char tr = 255;
    Color cupColor = {tr, tg, 30, 255};
    Color cupDark = {(unsigned char)(tr * 0.7f), (unsigned char)(tg * 0.7f), 20,
                     255};

    float cx = (float)GetScreenWidth() / 2.0f;
    float cupY = (float)screenCenterY - 132.0f;

    // Than cup (hinh thang)
    Vector2 v1 = {cx - 20, cupY - 22};
    Vector2 v2 = {cx + 20, cupY - 22};
    Vector2 v3 = {cx + 12, cupY + 6};
    Vector2 v4 = {cx - 12, cupY + 6};
    DrawTriangle(v2, v1, v4, cupColor);
    DrawTriangle(v2, v4, v3, cupColor);

    // Tay cam trai
    DrawRing({cx - 20, cupY - 10}, 8, 12, 90, 270, 16, cupDark);
    // Tay cam phai
    DrawRing({cx + 20, cupY - 10}, 8, 12, 270, 450, 16, cupDark);

    // Chan cup
    DrawRectangle((int)(cx - 3), (int)(cupY + 6), 6, 10, cupColor);

    // De cup
    DrawRectangle((int)(cx - 14), (int)(cupY + 16), 28, 5, cupColor);
    DrawRectangle((int)(cx - 16), (int)(cupY + 21), 32, 3, cupDark);
  }

  const char *title = "CHIEN THANG!";
  int titleSize = 64;
  int titleW = MeasureText(title, titleSize);
  float cp = victoryPulse * 2.0f;
  DrawText(title, GetScreenWidth() / 2 - titleW / 2 + 3, screenCenterY - 57 + 3, titleSize,
           Color{0, 0, 0, 180});
  DrawText(title, GetScreenWidth() / 2 - titleW / 2, screenCenterY - 57, titleSize,
           Color{(unsigned char)(200 + sinf(cp) * 55),
                 (unsigned char)(180 + sinf(cp + 2.09f) * 55),
                 (unsigned char)(50 + sinf(cp + 4.18f) * 50), 255});

  const char *scoreText = TextFormat("Diem so: %d", score);
  int sW = MeasureText(scoreText, 28);
  DrawText(scoreText, GetScreenWidth() / 2 - sW / 2, screenCenterY + 18, 28, Color{255, 230, 100, 240});

  const char *sub = "Ban da ha guc tat ca ke thu!";
  int subW = MeasureText(sub, 18);
  DrawText(sub, GetScreenWidth() / 2 - subW / 2, screenCenterY + 54, 18, Color{200, 240, 200, 200});

  float lineAlpha2 = 160.0f + sinf(victoryPulse) * 60.0f;
  DrawRectangle(GetScreenWidth() / 2 - 180, screenCenterY + 82, 360, 2,
                Color{255, 200, 50, (unsigned char)lineAlpha2});

  float alpha2 = 180.0f + sinf(victoryPulse) * 70.0f;
  float scale2 = 1.0f + sinf(victoryPulse) * 0.04f;
  const char *btnText = ">> ESC hoac ENTER de ve MENU <<";
  int btnSize = (int)(24 * scale2);
  int btnW2 = MeasureText(btnText, btnSize);
  int bx = GetScreenWidth() / 2 - btnW2 / 2 - 18, by = screenCenterY + 102, bw = btnW2 + 36,
      bh = btnSize + 20;
  DrawRectangleRounded({(float)bx, (float)by, (float)bw, (float)bh}, 0.4f, 8,
                       Color{30, 140, 60, (unsigned char)(alpha2 * 0.4f)});
  DrawRectangleLinesEx({(float)bx, (float)by, (float)bw, (float)bh}, 2,
                       Color{100, 255, 120, (unsigned char)alpha2});
  DrawText(btnText, GetScreenWidth() / 2 - btnW2 / 2, by + 10, btnSize,
           Color{255, 255, 255, (unsigned char)alpha2});

  const char *quit = "ESC: Thoat";
  int qW2 = MeasureText(quit, 16);
  DrawText(quit, GetScreenWidth() / 2 - qW2 / 2, screenCenterY + 178, 16, Color{160, 160, 160, 180});
}

// -------------------------------------------------------
// UPDATE
// -------------------------------------------------------
void Game::Update() {
  if (bgmPlaying) {
    UpdateMusicStream(bgm[currentBgm]);
  }
  UpdateBGM();

  if (state == GameState::MENU || state == GameState::INSTRUCTIONS) {
    menuBgY -= 120.0f * GetFrameTime();
    if (menuBgY < 0)
      menuBgY += bgTexture.height;
    menuPulse += GetFrameTime() * 3.0f;
    return;
  }
  if (state == GameState::GAME_OVER) {
    menuBgY -= 80.0f * GetFrameTime();
    if (menuBgY < 0)
      menuBgY += bgTexture.height;
    gameOverPulse += GetFrameTime() * 3.0f;
    return;
  }
  if (state == GameState::VICTORY) {
    menuBgY -= 60.0f * GetFrameTime();
    if (menuBgY < 0)
      menuBgY += bgTexture.height;
    victoryPulse += GetFrameTime() * 2.5f;
    return;
  }
  if (state == GameState::PAUSED)
    return;

  Vector2 playerPos = spaceship.GetPosition();
  spaceship.Update();

  for (auto &laser : spaceship.lasers)
    laser.Update();

  UpdateSpawner();
  UpdatePowerupSpawner();

  for (auto &enemy : enemies) {
    enemy.Update(playerPos);
    for (const auto &laser : enemy.lasers)
      enemyLasers.push_back(laser);
    enemy.lasers.clear();
  }

  for (auto &laser : enemyLasers)
    laser.Update();
  enemyLasers.erase(std::remove_if(enemyLasers.begin(), enemyLasers.end(),
                                   [](const Laser &l) { return !l.active; }),
                    enemyLasers.end());

  for (auto &p : powerups)
    p.Update();
  powerups.erase(std::remove_if(powerups.begin(), powerups.end(),
                                [](const Powerup &p) { return !p.active; }),
                 powerups.end());

  for (auto &p : particles) {
    p.position.x += p.velocity.x * GetFrameTime();
    p.position.y += p.velocity.y * GetFrameTime();
    p.life -= GetFrameTime() * 1.5f;
  }
  particles.erase(std::remove_if(particles.begin(), particles.end(),
                                 [](const Particle &p) { return p.life <= 0; }),
                  particles.end());

  if (cameraShakeTimer > 0.0f) {
    cameraShakeTimer -= GetFrameTime();
    if (cameraShakeTimer < 0.0f)
      cameraShakeTimer = 0.0f;
  }

  CheckCollisions();
  CheckEnemyLaserCollisions();
  CheckPowerupCollisions();

  for (auto &enemy : enemies) {
    if (!enemy.escapedBottom)
      continue;
    if (currentWave == 1 && !godMode) {
      spaceship.hp--;
      cameraShakeTimer = 0.3f;
      StopSound(sfxHurt);
      PlaySound(sfxHurt);
    }
  }

  enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                               [](const Enemy &e) { return !e.alive; }),
                enemies.end());

  bool allSpawned = (spawnedCount >= spawnQueue);
  bool allDead = enemies.empty();

  if (currentWave == 1 && allSpawned && allDead && !waveCleared) {
    waveCleared = true;
    StartWave(2);
  }

  if (currentWave == 2 && !octopusSpawned && spawnedCount >= 15) {
    enemies.push_back(
        Enemy({(GetScreenWidth() - 96) / 2.0f, -100}, EnemyType::OCTOPUS, octopusTexture));
    octopusSpawned = true;
    StopSound(sfxBoss);
    PlaySound(sfxBoss);
  }

  if (currentWave == 2 && octopusSpawned && allSpawned && allDead &&
      !waveCleared) {
    waveCleared = true;
    victoryPulse = 0.0f;
    menuBgY = bgY;
    StopSound(sfxWin);
    PlaySound(sfxWin);
    state = GameState::VICTORY;
    if (score > highScore) {
      highScore = score;
      SaveHighScore();
    }
  }

  bgY -= 200.0f * GetFrameTime();
  if (bgY < 0)
    bgY += bgTexture.height;

  if (spaceship.hp <= 0) {
    gameOverPulse = 0.0f;
    menuBgY = bgY;
    state = GameState::GAME_OVER;
    if (score > highScore) {
      highScore = score;
      SaveHighScore();
    }
    return;
  }

  DeleteInactiveLasers();
}

// -------------------------------------------------------
// DRAW
// -------------------------------------------------------
void Game::Draw() {
  if (state == GameState::MENU) {
    DrawMenu();
    return;
  }
  if (state == GameState::INSTRUCTIONS) {
    DrawInstructions();
    return;
  }
  if (state == GameState::GAME_OVER) {
    DrawGameOver();
    return;
  }
  if (state == GameState::VICTORY) {
    DrawVictory();
    return;
  }

  Camera2D camera = {0};
  camera.zoom = 1.0f;
  if (cameraShakeTimer > 0.0f) {
    camera.offset.x = (float)(rand() % 11 - 5);
    camera.offset.y = (float)(rand() % 11 - 5);
  }
  BeginMode2D(camera);

  // Draw the main background - correct scrolling slice rendering
  DrawScrollingBg(bgTexture, bgY);

  for (auto &p : powerups)
    p.Draw();
  spaceship.Draw();
  for (auto &laser : spaceship.lasers)
    laser.Draw();
  for (auto &enemy : enemies) {
    enemy.Draw();
    DrawHealthBar(enemy);
  }
  for (auto &laser : enemyLasers)
    laser.Draw();

  BeginBlendMode(BLEND_ADDITIVE);
  for (const auto &p : particles) {
    Color c = p.color;
    c.a = (unsigned char)(255 * p.life);
    DrawCircleV(p.position, p.size, c);
  }
  EndBlendMode();

  EndMode2D();
  DrawHUD();

  if (state == GameState::PAUSED)
    DrawPaused();
}

// -------------------------------------------------------
// USE BOMB
// -------------------------------------------------------
void Game::UseBomb() {
  PlaySound(sfxExplode); // THÊM DÒNG NÀY
  for (auto &e : enemies) {
    if (!e.alive || e.GetRect().y <= 0)
      continue;
    if (e.type != EnemyType::OCTOPUS) {
      e.alive = false;
      score += e.scoreValue;
      SpawnExplosion({e.GetRect().x + e.GetRect().width / 2,
                      e.GetRect().y + e.GetRect().height / 2},
                     ORANGE);
    } else {
      e.hp -= 20;
      if (e.hp <= 0) {
        e.alive = false;
        score += e.scoreValue;
        SpawnExplosion({e.GetRect().x + e.GetRect().width / 2,
                        e.GetRect().y + e.GetRect().height / 2},
                       ORANGE);
      } else {
        SpawnExplosion({e.GetRect().x + e.GetRect().width / 2,
                        e.GetRect().y + e.GetRect().height / 2},
                       PURPLE);
      }
    }
  }
  StopSound(sfxExplode);
  PlaySound(sfxExplode);
  cameraShakeTimer = 0.5f;
}

// -------------------------------------------------------
// INPUT
// -------------------------------------------------------
void Game::HandleInput() {
  bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  if (shiftDown && IsKeyPressed(KEY_G)) {
    godMode = !godMode;
    if (godMode) {
      if (spaceship.hp < 3)
        spaceship.hp = 3;
      StopSound(sfxPowerup);
      PlaySound(sfxPowerup);
    }
  }

  if (IsKeyPressed(KEY_M)) {
    static bool muted = false;
    muted = !muted;
    SetMusicVolume(bgm[currentBgm], muted ? 0.0f : 0.5f);
  }

  if (state == GameState::MENU) {
    if (IsKeyPressed(KEY_UP)) {
      currentMenuOption--;
      if (currentMenuOption < 0)
        currentMenuOption = 2;
    }
    if (IsKeyPressed(KEY_DOWN)) {
      currentMenuOption++;
      if (currentMenuOption > 2)
        currentMenuOption = 0;
    }

    if (IsKeyPressed(KEY_ENTER)) {
      if (currentMenuOption == 0) {
        isGameActive = true;
        state = GameState::PLAYING;
        score = 0;
        godMode = false;
        spaceship.Reset();
        enemyLasers.clear();
        powerups.clear();
        StartWave(1);
      } else if (currentMenuOption == 1) {
        if (isGameActive)
          state = GameState::PLAYING;
      } else if (currentMenuOption == 2) {
        state = GameState::INSTRUCTIONS;
      }
    }
    return;
  }

  if (state == GameState::INSTRUCTIONS) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
      state = GameState::MENU;
    return;
  }

  if (state == GameState::GAME_OVER) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
      isGameActive = false;
      state = GameState::MENU;
      bgY = 0.0f;
    }
    return;
  }

  if (state == GameState::VICTORY) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
      isGameActive = false;
      state = GameState::MENU;
      bgY = 0.0f;
    }
    return;
  }

  Rectangle pauseBtn = {(float)GetScreenWidth() - 50, 10, 40, 40};
  bool clickedPause = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                      CheckCollisionPointRec(GetMousePosition(), pauseBtn);

  if (state == GameState::PLAYING) {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE) || clickedPause) {
      state = GameState::PAUSED;
      return;
    }
  } else if (state == GameState::PAUSED) {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE) || clickedPause)
      state = GameState::PLAYING;
    else if (IsKeyPressed(KEY_ENTER)) {
      state = GameState::MENU;
      menuBgY = bgY;
    }
    return;
  }

  if (IsKeyDown(KEY_LEFT))
    spaceship.MoveLeft();
  else if (IsKeyDown(KEY_RIGHT))
    spaceship.MoveRight();
  if (IsKeyDown(KEY_UP))
    spaceship.MoveUp();
  else if (IsKeyDown(KEY_DOWN))
    spaceship.MoveDown();
  spaceship.FireLaser();

  if (IsKeyPressed(KEY_Q) && spaceship.inventory[0] > 0) {
    spaceship.inventory[0]--;
    spaceship.ApplyHeal();
    StopSound(sfxPowerup);
    PlaySound(sfxPowerup);
  }
  if (IsKeyPressed(KEY_W) && spaceship.inventory[1] > 0) {
    spaceship.inventory[1]--;
    spaceship.ApplyRapidFire();
    StopSound(sfxPowerup);
    PlaySound(sfxPowerup);
  }
  if (IsKeyPressed(KEY_E) && spaceship.inventory[2] > 0) {
    spaceship.inventory[2]--;
    spaceship.ApplyShield();
    StopSound(sfxPowerup);
    PlaySound(sfxPowerup);
  }
  if (IsKeyPressed(KEY_R) && spaceship.inventory[3] > 0) {
    spaceship.inventory[3]--;
    UseBomb();
  }
}

void Game::DeleteInactiveLasers() {
  spaceship.lasers.erase(
      std::remove_if(spaceship.lasers.begin(), spaceship.lasers.end(),
                     [](const Laser &l) { return !l.active; }),
      spaceship.lasers.end());
}

// -------------------------------------------------------
// VFX
// -------------------------------------------------------
void Game::SpawnExplosion(Vector2 pos, Color color) {
  StopSound(sfxExplode); // THÊM DÒNG NÀY
  PlaySound(sfxExplode); // THÊM DÒNG NÀY
  int count = 15 + rand() % 10;
  for (int i = 0; i < count; i++) {
    Particle p;
    p.position = pos;
    float angle = (float)(rand() % 360) * DEG2RAD;
    float speed = (float)(rand() % 150 + 50);
    p.velocity = {cosf(angle) * speed, sinf(angle) * speed};
    p.color = color;
    p.life = 1.0f;
    p.size = (float)(rand() % 4 + 2);
    particles.push_back(p);
  }
}
