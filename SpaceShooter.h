#pragma once
#ifndef SPACE_SHOOTER_H
#define SPACE_SHOOTER_H

#include "raylib.h"


// ---------------------------------------------------------
// Constants
// ---------------------------------------------------------
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int MAX_ENEMIES = 20;
const int MAX_BULLETS = 50;
const int MAX_LEVEL = 10;

const int STATE_MENU = 0;
const int STATE_PLAYING = 1;
const int STATE_GAME_OVER = 2;
const int STATE_WIN = 3;

// ---------------------------------------------------------
// Structs
// ---------------------------------------------------------
struct GameResources {
	Texture2D backgroundTexture;
    Texture2D playerTexture;
    Texture2D enemyTexture;
    Texture2D bulletTexture;

    Sound shootSound;
    Sound explodeSound;
    Sound gameOverSound;
	Sound winSound;
    Sound playerHitSound;
    Music gameTheme;

};
struct Player {
    float x, y;
    float speed;
    int width, height;
    int lives;
    bool isAlive;
};

struct Enemy {
    float x, y;
    float speed;
    int width, height;
    int health;   // How many hits needed to destroy this enemy
    bool active;
};

struct Bullet {
    float x, y;
    float speed;
    int width, height;
    bool active;
};

struct GameState {
    int score;
    int level;          // 1..5
    int highScore;      // Only in memory (no file)
    int hitsToKill;     // Enemy health for current wave
    bool gameOver;
    bool gameWon;
    int gameState;      // STATE_MENU, STATE_PLAYING, etc.
};

// ---------------------------------------------------------
// Function declarations (prototypes)
// ---------------------------------------------------------

void InitWindowAndResources(GameResources& res);
void UnloadResourcesAndCloseWindow(GameResources& res);

void InitGame(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets);


void RunGameLoop(GameState& game, Player& player, Enemy enemies[],
    int& enemyCount, Bullet bullets[], int maxBullets, const GameResources& res);;

void DrawStartScreen(const GameState& game);
void HandleStartScreenInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets);

void DrawGameOverScreen(const GameState& game);
void HandleGameOverInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res);

void DrawWinScreen(const GameState& game);
void HandleWinScreenInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res);

void UpdateGame(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res);

void DrawGame(const GameState& game, const Player& player,
    const Enemy enemies[], int enemyCount,
    const Bullet bullets[], int maxBullets, const GameResources& res);


void InitPlayer(Player& player);
void InitBullets(Bullet bullets[], int maxBullets);
void InitEnemiesForLevel(const GameState& game,
    Enemy enemies[], int& enemyCount);

bool AreAllEnemiesDestroyed(const Enemy enemies[], int enemyCount);

void UpdatePlayer(Player& player);
void HandlePlayerShooting(const Player& player,
    Bullet bullets[], int maxBullets, const GameResources& res);

void UpdateBullets(Bullet bullets[], int maxBullets);
void UpdateEnemies(Enemy enemies[], int enemyCount);

bool RectanglesOverlap(float x1, float y1, int w1, int h1,
    float x2, float y2, int w2, int h2);

void CheckBulletEnemyCollisions(Bullet bullets[], int maxBullets,
    Enemy enemies[], int enemyCount,
    GameState& game, const GameResources& res);

bool CheckEnemyPlayerCollisions(const Enemy enemies[], int enemyCount,
    const Player& player);

void HandlePlayerHit(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res);

void DrawHUD(const GameState& game, const Player& player);

void UpdateScoreAndLevel(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res);

void ResetLevel(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets);

void ResetGameToLevel1(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets);

void SaveGame(const GameState& game, const Player& player);
void LoadGame(GameState& game, Player& player);

#endif // SPACE_SHOOTER_H
