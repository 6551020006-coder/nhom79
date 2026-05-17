#pragma once
#include <raylib.h>
#include "boss.hpp"
#include "powerup.hpp"
#include "spaceship.hpp"
#include <string>
#include <vector>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life; // 1.0f -> 0.0f
    float size;
};

enum class GameState {
  MENU,
  PLAYING,
  PAUSED,
  GAME_OVER,
  VICTORY,
  INSTRUCTIONS
};

class Game {
public:
  Game();
  ~Game();
  void Draw();
  void Update();
  void HandleInput();
  GameState GetState() const { return state; }

  std::vector<Enemy> enemies;

private:
  // Menu
  void DrawMenu();
  void DrawInstructions();

  // Game Over
  void DrawGameOver();

  // Victory
  void DrawVictory();

  // Pause
  void DrawPaused();

  // Bomb Logic
  void UseBomb();

  // Wave management
  void DeleteInactiveLasers();
  void UpdateSpawner();
  void StartWave(int wave);
  void UpdatePowerupSpawner();

  // Collision
  void CheckCollisions();
  void CheckEnemyLaserCollisions();
  void CheckPowerupCollisions();

  // Enemy health bar
  void DrawHealthBar(const Enemy &enemy);

  // HUD
  void DrawHUD();

  // VFX
  void SpawnExplosion(Vector2 pos, Color color);

  // -------------------------------------------------------
  std::vector<Laser> enemyLasers;
  std::vector<Powerup> powerups;
  std::vector<Particle> particles;
  Spaceship spaceship;
  float cameraShakeTimer;

  // Textures
  Texture2D droneTexture;
  Texture2D scoutTexture;
  Texture2D octopusTexture;
  Texture2D bgTexture;

  // Powerup textures
  Texture2D texShield;
  Texture2D texRapidFire;
  Texture2D texHeal;
  Texture2D texBomb;

  // Wave state
  int currentWave;
  int spawnQueue;
  int spawnedCount;
  double lastSpawnTime;
  float spawnInterval;
  bool waveCleared;
  bool octopusSpawned;
  int maxOnScreen;

  // Background scroll
  float bgY;

  // Score
  int score;
  int highScore;

  // File IO for score
  void LoadHighScore();
  void SaveHighScore();

  // Game state
  GameState state;
  float menuPulse;
  float menuBgY;
  float gameOverPulse;
  float victoryPulse;

  // Menu navigation
  bool isGameActive;
  int currentMenuOption;

  // God mode (Shift+G)
  bool godMode;

  // Powerup tren troi roi xuong
  int powerupsDropped;
  int maxPowerupsPerWave;
  double lastPowerupDropTime;
  float nextPowerupInterval;

  // --- Am thanh truc tiep (khong dung AudioManager) ---
  Music bgm[3];
  int   currentBgm;
  bool  bgmPlaying;

  Sound sfxLaser;
  Sound sfxExplode;
  Sound sfxEnemyLaser;
  Sound sfxHit;
  Sound sfxPowerup;
  Sound sfxHurt;
  Sound sfxBoss;
  Sound sfxWin;

  void PlayBGM();
  void StopBGM();
  void UpdateBGM();
};
