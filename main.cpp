#include <iostream>
#include <fstream>
#include <raylib.h>
using namespace std;

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int MAX_ENEMIES = 30;
const int MAX_BULLETS = 10;
const int MAX_BOSS_BULLETS = 20;
const int MAX_LEVEL = 5;

// Boss constants
const int BOSS_WIDTH = 200;
const int BOSS_HEIGHT = 200;
const float BOSS_SPEED = 2.0f;
const int BOSS_INITIAL_HEALTH = 100;

// Game states
enum GameStateEnum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_WIN,
    STATE_BOSS_FIGHT
};

// Structures for game entities and state
struct Player {
    float x;
    float y;
    int width;
    int height;
    float speed;
    int lives;
    bool isAlive;
};

struct Enemy {
    float x;
    float y;
    int width;
    int height;
    float speed;
    int health;
    bool active;
};

struct Bullet {
    float x;
    float y;
    int width;
    int height;
    float speed;
    bool active;
};

struct Boss {
    float x;
    float y;
    int width;
    int height;
    float speed;
    int health;
    bool active;
    float shootTimer;
};

struct GameState {
    int score;
    int level;
    int highScore;
    int hitsToKill;
    bool gameOver;
    bool gameWon;
    GameStateEnum gameState;
    bool bossActive;
};

struct GameResources {
    Texture2D playerTexture;
    Texture2D enemyTexture;
    Texture2D bulletTexture;
    Texture2D backgroundTexture;
    Texture2D bossTexture;
    Texture2D bossBulletTexture;

    Sound shootSound;
    Sound explodeSound;
    Sound gameOverSound;
    Sound winSound;
    Sound playerHitSound;
    Music gameTheme;
};

// -----------------------------------------------------------------------------
// FUNCTION PROTOTYPES (ALL PARAMETERS)
// -----------------------------------------------------------------------------

// Window / resources
void InitWindowAndResources(GameResources& res);
void UnloadResourcesAndCloseWindow(GameResources& res);

// Game initialization
void InitPlayer(Player& player);
void InitBullets(Bullet bullets[], int maxBullets);
void InitBoss(Boss& boss);
void InitBossBullets(Bullet bossBullets[], int maxBossBullets);
bool OverlapsAnyPreviousEnemy(const Enemy enemies[], int countSoFar, float x, float y, int w, int h);
void InitEnemiesForLevel(const GameState& game, Enemy enemies[], int& enemyCount);
void InitGame(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets);

// Main game loop
void RunGameLoop(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);

// Screens: Start / Game Over / Win
void DrawStartScreen(const GameState& game);
void HandleStartScreenInput(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets);
void DrawGameOverScreen(const GameState& game);
void HandleGameOverInput(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);
void DrawWinScreen(const GameState& game);
void HandleWinScreenInput(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);

// Game update & drawing (PLAYING/BOSS state)
void UpdateGame(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);
void DrawGame(const GameState& game, const Player& player, const Enemy enemies[], int enemyCount, const Bullet bullets[], int maxBullets, const Boss& boss, const Bullet bossBullets[], int maxBossBullets, const GameResources& res);

// Player movement + shooting
void UpdatePlayer(Player& player);
void HandlePlayerShooting(const Player& player, Bullet bullets[], int maxBullets, const GameResources& res);

// Bullets & Enemies & Boss
void UpdateBullets(Bullet bullets[], int maxBullets);
void UpdateEnemies(Enemy enemies[], int enemyCount);
void UpdateBoss(Boss& boss);
void HandleBossShooting(Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);
void UpdateBossBullets(Bullet bossBullets[], int maxBossBullets);

// Collisions & lives
bool RectanglesOverlap(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
void CheckBulletEnemyCollisions(Bullet bullets[], int maxBullets, Enemy enemies[], int enemyCount, GameState& game, const GameResources& res);
bool CheckEnemyPlayerCollisions(const Enemy enemies[], int enemyCount, const Player& player);
void CheckBulletBossCollisions(Bullet bullets[], int maxBullets, Boss& boss, GameState& game, const GameResources& res);
bool CheckBossPlayerCollision(const Boss& boss, const Player& player);
bool CheckBossBulletPlayerCollisions(const Bullet bossBullets[], int maxBossBullets, const Player& player);
void HandlePlayerHit(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);

// HUD, scoring, level progression
void DrawHUD(const GameState& game, const Player& player, const Boss& boss);
void UpdateScoreAndLevel(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res);
void ResetLevel(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets);
void ResetGameToLevel1(GameState& game, Player& player, Enemy enemies[], int& enemyCount, Bullet bullets[], int maxBullets, Boss& boss, Bullet bossBullets[], int maxBossBullets);
bool AreAllEnemiesDestroyed(const Enemy enemies[], int enemyCount);

// Save/Load
void SaveGame(const GameState& game, const Player& player, const Boss& boss);
void LoadGame(GameState& game, Player& player, Boss& boss);


// ---------------------------------------------------------
// MAIN FUNCTION 
// ---------------------------------------------------------
int main()
{
    GameResources resources = { 0 };
    InitWindowAndResources(resources);
    PlayMusicStream(resources.gameTheme);
    SetMusicVolume(resources.gameTheme, 0.2f);

    GameState game;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Bullet bullets[MAX_BULLETS];
    Boss boss;
    Bullet bossBullets[MAX_BOSS_BULLETS];
    int enemyCount = 0;

    
    InitGame(game, player, enemies, enemyCount, bullets, MAX_BULLETS, boss, bossBullets, MAX_BOSS_BULLETS);
    RunGameLoop(game, player, enemies, enemyCount, bullets, MAX_BULLETS, boss, bossBullets, MAX_BOSS_BULLETS, resources);

    UnloadResourcesAndCloseWindow(resources);
    return 0;
}

// ---------------------------------------------------------
// Window / resources
// ---------------------------------------------------------
void InitWindowAndResources(GameResources& res)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Shooter - PF Project");
    res.bulletTexture = LoadTexture("bullet.png");
    res.enemyTexture = LoadTexture("enemy.png");
    res.playerTexture = LoadTexture("shooter.png");
    res.backgroundTexture = LoadTexture("background.png");
    res.bossTexture = LoadTexture("boss.png");
    res.bossBulletTexture = LoadTexture("bullet.png");

    InitAudioDevice();
    res.shootSound = LoadSound("shoot.wav");
    res.explodeSound = LoadSound("explosion.wav");
    res.gameOverSound = LoadSound("gameover.wav");
    res.winSound = LoadSound("win.wav");
    res.playerHitSound = LoadSound("hit.wav");
    res.gameTheme = LoadMusicStream("theme.mp3");

    SetTargetFPS(60);
    SetExitKey(0);
}

void UnloadResourcesAndCloseWindow(GameResources& res)
{
  
    UnloadTexture(res.bulletTexture);
    UnloadTexture(res.enemyTexture);
    UnloadTexture(res.playerTexture);
    UnloadTexture(res.backgroundTexture);
    UnloadTexture(res.bossTexture);
    UnloadTexture(res.bossBulletTexture);


    UnloadSound(res.shootSound);
    UnloadSound(res.explodeSound);
    UnloadSound(res.gameOverSound);
    UnloadSound(res.winSound);
    UnloadSound(res.playerHitSound);
    UnloadMusicStream(res.gameTheme);

    CloseAudioDevice();
    CloseWindow();
}

// ---------------------------------------------------------
// Game initialization
// ---------------------------------------------------------
void InitPlayer(Player& player)
{
    player.width = 60;
    player.height = 60;
    player.x = SCREEN_WIDTH / 2.0f - player.width / 2.0f;
    player.y = SCREEN_HEIGHT - 60.0f;
    player.speed = 5.0f;
    player.lives = 3;
    player.isAlive = true;
}

void InitBullets(Bullet bullets[], int maxBullets)
{
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].active = false;
        bullets[i].width = 30;
        bullets[i].height = 30;
        bullets[i].speed = 8.0f;
        bullets[i].x = 0;
        bullets[i].y = 0;
    }
}

void InitBoss(Boss& boss)
{
    boss.width = BOSS_WIDTH;
    boss.height = BOSS_HEIGHT;
    boss.x = SCREEN_WIDTH / 2.0f - boss.width / 2.0f;
    boss.y = 50.0f;
    boss.speed = BOSS_SPEED;
    boss.health = BOSS_INITIAL_HEALTH;
    boss.active = false;
    boss.shootTimer = 1.0f;
}

void InitBossBullets(Bullet bossBullets[], int maxBossBullets)
{
    for (int i = 0; i < maxBossBullets; i++) {
        bossBullets[i].active = false;
        bossBullets[i].width = 30;
        bossBullets[i].height = 30;
        bossBullets[i].speed = 6.0f;
        bossBullets[i].x = 0;
        bossBullets[i].y = 0;
    }
}

bool OverlapsAnyPreviousEnemy(const Enemy enemies[], int countSoFar,
    float x, float y, int w, int h)
{
    for (int i = 0; i < countSoFar; i++) {
        if (!enemies[i].active) continue;

        if (RectanglesOverlap(x, y, w, h,
            enemies[i].x, enemies[i].y,
            enemies[i].width, enemies[i].height)) {
            return true;
        }
    }
    return false;
}

void InitEnemiesForLevel(const GameState& game,
    Enemy enemies[], int& enemyCount)
{
    int level = game.level;

   
    enemyCount = 3 + level * 3;
    if (enemyCount > MAX_ENEMIES) enemyCount = MAX_ENEMIES;

    float baseSpeed = 1.0f + level * 0.3f;

    int centerX = SCREEN_WIDTH / 2;
    int halfRange = 250;
    int minX = centerX - halfRange;
    if (minX < 0) minX = 0;

    int maxX = centerX + halfRange;
    if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;

    for (int i = 0; i < enemyCount; i++) {
        enemies[i].width = 80;
        enemies[i].height = 80;

        float x = 0;
        float y = 0;
        const int MAX_TRIES = 30;
        int tries = 0;

        do {
            x = (float)GetRandomValue(minX, maxX - enemies[i].width);
            y = (float)GetRandomValue(60, 220);
            tries++;
        } while (OverlapsAnyPreviousEnemy(enemies, i, x, y,
            enemies[i].width, enemies[i].height)
            && tries < MAX_TRIES);

        enemies[i].x = x;
        enemies[i].y = y;

        float randomOffset = (float)GetRandomValue(-3, 3) * 0.1f;
        enemies[i].speed = baseSpeed + randomOffset;
        if (enemies[i].speed < 0.5f) enemies[i].speed = 0.5f;

        enemies[i].health = game.hitsToKill;
        enemies[i].active = true;
    }

    for (int i = enemyCount; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        enemies[i].health = 0;
    }
}

void InitGame(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets)
{
    game.score = 0;
    game.level = 1;
    game.highScore = 0;
    game.hitsToKill = 1;
    game.gameOver = false;
    game.gameWon = false;
    game.gameState = STATE_MENU;
    game.bossActive = false;

    InitPlayer(player);
    player.lives = 3;

    

    InitBullets(bullets, maxBullets);
    InitEnemiesForLevel(game, enemies, enemyCount);
    InitBoss(boss);
    InitBossBullets(bossBullets, maxBossBullets);
}

// ---------------------------------------------------------
// Main game loop
// ---------------------------------------------------------
void RunGameLoop(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    while (!WindowShouldClose())
    {
        UpdateMusicStream(res.gameTheme);
        if (IsKeyPressed(KEY_ESCAPE)) {
            SaveGame(game, player, boss);
            break;
        }

        
        if (game.gameState == STATE_MENU) {
           
            HandleStartScreenInput(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, MAX_BOSS_BULLETS);
        }
        else if (game.gameState == STATE_PLAYING || game.gameState == STATE_BOSS_FIGHT) {
            UpdateGame(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, MAX_BOSS_BULLETS, res);
        }
        else if (game.gameState == STATE_GAME_OVER) {
           
            HandleGameOverInput(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, MAX_BOSS_BULLETS, res);
        }
        else if (game.gameState == STATE_WIN) {
           
            HandleWinScreenInput(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, MAX_BOSS_BULLETS, res);
        }

     
        BeginDrawing();
        ClearBackground(BLACK);

        if (game.gameState == STATE_MENU) {
            DrawStartScreen(game);
        }
        else if (game.gameState == STATE_PLAYING || game.gameState == STATE_BOSS_FIGHT) {
            DrawGame(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, MAX_BOSS_BULLETS, res);
        }
        else if (game.gameState == STATE_GAME_OVER) {
            DrawGameOverScreen(game);
        }
        else if (game.gameState == STATE_WIN) {
            DrawWinScreen(game);
        }

        EndDrawing();
    }
}

// ---------------------------------------------------------
// Screens: Start / Game Over / Win
// ---------------------------------------------------------
void DrawStartScreen(const GameState& game)
{
    const char* title = "SPACE SHOOTER";
    int titleWidth = MeasureText(title, 40);

    DrawText(title,
        SCREEN_WIDTH / 2 - titleWidth / 2, 30,
        40, RAYWHITE);

    int y = 90;

    // Controls
    DrawText("CONTROLS", 60, y, 24, YELLOW);
    y += 30;
    DrawText("- Move Left  : LEFT ARROW", 60, y, 20, LIGHTGRAY);  y += 24;
    DrawText("- Move Right : RIGHT ARROW", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- Shoot      : SPACE", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- Save & Quit: ESC", 60, y, 20, LIGHTGRAY); y += 36;

    // Rules
    DrawText("GAME RULES", 60, y, 24, YELLOW);
    y += 30;
    DrawText("- You start with 3 lives.", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- Colliding with an enemy costs 1 life.", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- Each destroyed enemy gives 1 point.", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- To reach the next level: score >= level * 10.", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- If you destroy all enemies but don't have", 60, y, 20, LIGHTGRAY); y += 20;
    DrawText("  enough score, a new, tougher wave spawns.", 60, y, 20, LIGHTGRAY); y += 24;
    DrawText("- There are 5 levels, then the BOSS FIGHT starts.", 60, y, 20, LIGHTGRAY); y += 32;

    // Menu options
    DrawText("MENU", 60, y, 24, YELLOW);
    y += 30;
    DrawText("Press N      - New Game (start from Level 1)", 60, y, 20, GREEN); y += 24;
    DrawText("Press L      - Load Saved Game (if available)", 60, y, 20, GREEN); y += 24;
    DrawText("Press ENTER - Same as New Game", 60, y, 20, GREEN); y += 24;

    // High score display
    DrawText(TextFormat("Best Score (this run): %d", game.highScore),
        10, 10, 20, SKYBLUE);
}


void HandleStartScreenInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_N)) {
        ResetGameToLevel1(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets);
        game.gameState = STATE_PLAYING;
    }
    else if (IsKeyPressed(KEY_L)) {
        LoadGame(game, player, boss);

        InitBullets(bullets, maxBullets);
        InitBossBullets(bossBullets, maxBossBullets);

        if (game.bossActive) {
            boss.active = true;
            game.gameState = STATE_BOSS_FIGHT;
        }
        else {
            InitEnemiesForLevel(game, enemies, enemyCount);
            player.isAlive = true;
            game.gameOver = false;
            game.gameWon = false;
            game.gameState = STATE_PLAYING;
        }
    }
}

void DrawGameOverScreen(const GameState& game)
{
    const char* msg = "GAME OVER";
    int msgWidth = MeasureText(msg, 40);
    DrawText(msg,
        SCREEN_WIDTH / 2 - msgWidth / 2, 160,
        40, RED);

    DrawText(TextFormat("Score: %d", game.score),
        260, 220, 20, RAYWHITE);

    DrawText(TextFormat("Best This Run: %d", game.highScore),
        260, 250, 20, GREEN);

    DrawText("Press ENTER to restart from Level 1",
        160, 320, 20, YELLOW);
}


void HandleGameOverInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    if (IsKeyPressed(KEY_ENTER)) {
        ResetGameToLevel1(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets);
        game.gameState = STATE_PLAYING;
        PlayMusicStream(res.gameTheme);
    }
}

void DrawWinScreen(const GameState& game)
{
    const char* msg = "YOU WIN!";
    int msgWidth = MeasureText(msg, 40);
    DrawText(msg,
        SCREEN_WIDTH / 2 - msgWidth / 2, 160,
        40, GREEN);

    DrawText(TextFormat("Final Score: %d", game.score),
        260, 220, 20, RAYWHITE);

    DrawText(TextFormat("Best This Run: %d", game.highScore),
        260, 250, 20, GREEN);

    DrawText("Press ENTER to play again from Level 1",
        130, 320, 20, YELLOW);
}


void HandleWinScreenInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    if (IsKeyPressed(KEY_ENTER)) {
        ResetGameToLevel1(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets);
        game.gameState = STATE_PLAYING;
        PlayMusicStream(res.gameTheme);
    }
}

// ---------------------------------------------------------
// Game update & drawing (PLAYING state)
// ---------------------------------------------------------
void UpdateGame(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    UpdatePlayer(player);
    HandlePlayerShooting(player, bullets, maxBullets, res);
    UpdateBullets(bullets, maxBullets);

    if (game.gameState == STATE_PLAYING) {
        UpdateEnemies(enemies, enemyCount);
        CheckBulletEnemyCollisions(bullets, maxBullets, enemies, enemyCount, game, res);
        if (CheckEnemyPlayerCollisions(enemies, enemyCount, player)) {
            HandlePlayerHit(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets, res);
        }
    }
    else if (game.gameState == STATE_BOSS_FIGHT) {
        UpdateBoss(boss);
        HandleBossShooting(boss, bossBullets, maxBossBullets, res);
        UpdateBossBullets(bossBullets, maxBossBullets);

        CheckBulletBossCollisions(bullets, maxBullets, boss, game, res);

        if (CheckBossPlayerCollision(boss, player) || CheckBossBulletPlayerCollisions(bossBullets, maxBossBullets, player)) {
            HandlePlayerHit(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets, res);
        }
    }

    UpdateScoreAndLevel(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets, res);
}

void DrawGame(const GameState& game, const Player& player,
    const Enemy enemies[], int enemyCount,
    const Bullet bullets[], int maxBullets,
    const Boss& boss, const Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    // Draw Background
    if (res.backgroundTexture.id > 0) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)res.backgroundTexture.width, (float)res.backgroundTexture.height };
        Rectangle destRec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        Vector2 origin = { 0, 0 };
        DrawTexturePro(res.backgroundTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    }

    // Draw Player
    if (player.isAlive) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)res.playerTexture.width, (float)res.playerTexture.height };
        Rectangle destRec = { player.x, player.y, player.width, player.height };
        Vector2 origin = { 0, 0 };
        DrawTexturePro(res.playerTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    }

    // Draw Enemies 
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active && game.gameState == STATE_PLAYING) {
            if (res.enemyTexture.id > 0) {
                Rectangle source = { 0, 0, (float)res.enemyTexture.width, (float)res.enemyTexture.height };
                Rectangle dest = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
                Vector2 origin = { 0, 0 };
                DrawTexturePro(res.enemyTexture, source, dest, origin, 0.0f, WHITE);
            }
        }
    }

    // Draw Bullets (Player)
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {
            if (res.bulletTexture.id > 0) {
                Rectangle source = { 0, 0, (float)res.bulletTexture.width, (float)res.bulletTexture.height };
                Rectangle dest = { bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height };
                Vector2 origin = { 0, 0 };
                DrawTexturePro(res.bulletTexture, source, dest, origin, 0.0f, WHITE);
            }
            else {
                DrawRectangle((int)bullets[i].x, (int)bullets[i].y, (int)bullets[i].width, (int)bullets[i].height, YELLOW);
            }
        }
    }

    // Draw Boss
    if (boss.active) {
        if (res.bossTexture.id > 0) {
            Rectangle source = { 0, 0, (float)res.bossTexture.width, (float)res.bossTexture.height };
            Rectangle dest = { boss.x, boss.y, boss.width, boss.height };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(res.bossTexture, source, dest, origin, 0.0f, WHITE);
        }
        else {
            DrawRectangle((int)boss.x, (int)boss.y, (int)boss.width, (int)boss.height, PURPLE);
        }
    }

    // Draw Boss Bullets
    for (int i = 0; i < maxBossBullets; i++) {
        if (bossBullets[i].active) {
            if (res.bossBulletTexture.id > 0) {
                Rectangle source = { 0, 0, (float)res.bossBulletTexture.width, (float)res.bossBulletTexture.height };
                Rectangle dest = { bossBullets[i].x, bossBullets[i].y, bossBullets[i].width, bossBullets[i].height };
                Vector2 origin = { 0, 0 };
                DrawTexturePro(res.bossBulletTexture, source, dest, origin, 0.0f, RED);
            }
            else {
                DrawRectangle((int)bossBullets[i].x, (int)bossBullets[i].y, (int)bossBullets[i].width, (int)bossBullets[i].height, RED);
            }
        }
    }

    DrawHUD(game, player, boss);
}
bool AreAllEnemiesDestroyed(const Enemy enemies[], int enemyCount)
{
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------
// Player movement + shooting
// ---------------------------------------------------------
void UpdatePlayer(Player& player)
{
    if (!player.isAlive) return;

    if (IsKeyDown(KEY_LEFT)) {
        player.x -= player.speed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player.x += player.speed;
    }

   
    if (player.x < 0) player.x = 0;
    if (player.x + player.width > SCREEN_WIDTH) {
        player.x = SCREEN_WIDTH - player.width;
    }
}

void HandlePlayerShooting(const Player& player,
    Bullet bullets[], int maxBullets, const GameResources& res)
{
    if (IsKeyPressed(KEY_SPACE)) {
        for (int i = 0; i < maxBullets; i++) {
            if (!bullets[i].active) {
                PlaySound(res.shootSound);

                bullets[i].active = true;
                bullets[i].x = player.x + player.width / 2.0f - bullets[i].width / 2.0f;
                bullets[i].y = player.y - bullets[i].height;
                break;
            }
        }
    }
}

// ---------------------------------------------------------
// Bullets & Enemies & Boss
// ---------------------------------------------------------
void UpdateBullets(Bullet bullets[], int maxBullets)
{
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {
            bullets[i].y -= bullets[i].speed;
            if (bullets[i].y + bullets[i].height < 0) {
                bullets[i].active = false;
            }
        }
    }
}

void UpdateEnemies(Enemy enemies[], int enemyCount)
{
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            enemies[i].y += enemies[i].speed;

            if (enemies[i].y > SCREEN_HEIGHT) {
                enemies[i].y = -enemies[i].height;
            }
        }
    }
}

void UpdateBoss(Boss& boss)
{
    if (!boss.active) return;

    boss.x += boss.speed;

    if (boss.x + boss.width > SCREEN_WIDTH || boss.x < 0) {
        boss.speed *= -1;
    }
}

void HandleBossShooting(Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    if (!boss.active) return;

    boss.shootTimer -= GetFrameTime();

    if (boss.shootTimer <= 0) {
        PlaySound(res.shootSound);

        boss.shootTimer = 1.0f + (float)boss.health / BOSS_INITIAL_HEALTH * 0.5f;
        if (boss.shootTimer < 0.3f) boss.shootTimer = 0.3f;

        int bulletsFired = 0;
        for (int i = 0; i < maxBossBullets && bulletsFired < 3; i++) {
            if (!bossBullets[i].active) {
                bossBullets[i].active = true;
                bossBullets[i].x = boss.x + boss.width / 2.0f - bossBullets[i].width / 2.0f;
                bossBullets[i].y = boss.y + boss.height;

                if (bulletsFired == 0) bossBullets[i].x -= 15;
                if (bulletsFired == 2) bossBullets[i].x += 15;

                bulletsFired++;
            }
        }
    }
}

void UpdateBossBullets(Bullet bossBullets[], int maxBossBullets)
{
    for (int i = 0; i < maxBossBullets; i++) {
        if (bossBullets[i].active) {
            bossBullets[i].y += bossBullets[i].speed;
            if (bossBullets[i].y > SCREEN_HEIGHT) {
                bossBullets[i].active = false;
            }
        }
    }
}

// ---------------------------------------------------------
// Collisions & lives
// ---------------------------------------------------------
bool RectanglesOverlap(float x1, float y1, int w1, int h1,
    float x2, float y2, int w2, int h2)
{
    if (x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2) {
        return true;
    }
    return false;
}

void CheckBulletEnemyCollisions(Bullet bullets[], int maxBullets,
    Enemy enemies[], int enemyCount,
    GameState& game, const GameResources& res)
{
    for (int i = 0; i < maxBullets; i++) {
        if (!bullets[i].active) continue;

        for (int j = 0; j < enemyCount; j++) {
            if (!enemies[j].active) continue;

            if (RectanglesOverlap(bullets[i].x, bullets[i].y,
                bullets[i].width, bullets[i].height,
                enemies[j].x, enemies[j].y,
                enemies[j].width, enemies[j].height)) {

                bullets[i].active = false;
                enemies[j].health--;

                PlaySound(res.explodeSound);

                if (enemies[j].health <= 0) {
                    enemies[j].active = false;
                    game.score += 1;
                    if (game.score > game.highScore) {
                        game.highScore = game.score;
                    }
                }
                break;
            }
        }
    }
}

void CheckBulletBossCollisions(Bullet bullets[], int maxBullets,
    Boss& boss, GameState& game, const GameResources& res)
{
    if (!boss.active) return;

    for (int i = 0; i < maxBullets; i++) {
        if (!bullets[i].active) continue;

        if (RectanglesOverlap(bullets[i].x, bullets[i].y,
            bullets[i].width, bullets[i].height,
            boss.x, boss.y,
            boss.width, boss.height)) {

            bullets[i].active = false;
            boss.health--;

            PlaySound(res.explodeSound);

            if (boss.health <= 0) {
                boss.active = false;
                game.score += 10;
                game.gameWon = true;
                game.gameState = STATE_WIN;
                StopMusicStream(res.gameTheme);
                PlaySound(res.winSound);
            }
            break;
        }
    }
}

bool CheckEnemyPlayerCollisions(const Enemy enemies[], int enemyCount,
    const Player& player)
{
    if (!player.isAlive) return false;

    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;

        if (RectanglesOverlap(player.x, player.y,
            player.width, player.height,
            enemies[i].x, enemies[i].y,
            enemies[i].width, enemies[i].height)) {
            return true;
        }
    }
    return false;
}

bool CheckBossPlayerCollision(const Boss& boss, const Player& player)
{
    if (!boss.active || !player.isAlive) return false;

    return RectanglesOverlap(player.x, player.y,
        player.width, player.height,
        boss.x, boss.y,
        boss.width, boss.height);
}

bool CheckBossBulletPlayerCollisions(const Bullet bossBullets[], int maxBossBullets,
    const Player& player)
{
    if (!player.isAlive) return false;

    for (int i = 0; i < maxBossBullets; i++) {
        if (!bossBullets[i].active) continue;

        if (RectanglesOverlap(player.x, player.y,
            player.width, player.height,
            bossBullets[i].x, bossBullets[i].y,
            bossBullets[i].width, bossBullets[i].height)) {

            
            ((Bullet*)bossBullets)[i].active = false;
            return true;
        }
    }
    return false;
}


void HandlePlayerHit(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    player.lives--;
    if (player.lives > 0) {
        PlaySound(res.playerHitSound);
    }
    if (player.lives <= 0) {
        StopMusicStream(res.gameTheme);
        PlaySound(res.gameOverSound);
        player.isAlive = false;
        game.gameOver = true;
        game.gameState = STATE_GAME_OVER;
        return;
    }

    // Reset player position
    player.isAlive = true;
    player.x = SCREEN_WIDTH / 2.0f - player.width / 2.0f;
    player.y = SCREEN_HEIGHT - 60.0f;

    // Clear player bullets
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].active = false;
    }

    // Clear boss bullets
    for (int i = 0; i < maxBossBullets; i++) {
        bossBullets[i].active = false;
    }


    if (game.gameState == STATE_PLAYING) {
        InitEnemiesForLevel(game, enemies, enemyCount);
    }
    else if (game.gameState == STATE_BOSS_FIGHT) {
      
        boss.x = SCREEN_WIDTH / 2.0f - boss.width / 2.0f;
        boss.y = 50.0f;
        boss.shootTimer = 1.0f; // Reset shoot timer
    }
}
// ---------------------------------------------------------
// HUD, scoring, level progression
// ---------------------------------------------------------
void DrawHUD(const GameState& game, const Player& player, const Boss& boss)
{
    DrawText(TextFormat("Score: %d", game.score),
        10, 10, 20, RAYWHITE);

    if (game.gameState == STATE_BOSS_FIGHT) {
        DrawText("Level: BOSS", 10, 35, 20, RED);
    }
    else {
        DrawText(TextFormat("Level: %d", game.level),
            10, 35, 20, RAYWHITE);
    }

    DrawText(TextFormat("Lives: %d", player.lives),
        10, 60, 20, RAYWHITE);

    DrawText(TextFormat("Best: %d", game.highScore),
        SCREEN_WIDTH - 180, 10, 20, GREEN);

    if (boss.active) {
        DrawText(TextFormat("BOSS HEALTH: %d", boss.health),
            SCREEN_WIDTH / 2 - 100, 10, 20, RED);
    }
}

void UpdateScoreAndLevel(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets, const GameResources& res)
{
    bool allDead = AreAllEnemiesDestroyed(enemies, enemyCount);

    if (game.score >= game.level * 10 && game.gameState == STATE_PLAYING) {
        game.level++;

        if (game.level > MAX_LEVEL) {
            game.hitsToKill = 1;
            InitBoss(boss);
            boss.active = true;
            game.bossActive = true;
            game.gameState = STATE_BOSS_FIGHT;
        }
        else {
            game.hitsToKill = 1;
            ResetLevel(game, player, enemies, enemyCount, bullets, maxBullets, boss, bossBullets, maxBossBullets);
        }
    }
    else if (allDead && game.gameState == STATE_PLAYING) {
        game.hitsToKill++;
        InitEnemiesForLevel(game, enemies, enemyCount);
    }
}

void ResetLevel(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets)
{
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].active = false;
    }

    for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
        bossBullets[i].active = false;
    }
    InitBoss(boss);

    InitEnemiesForLevel(game, enemies, enemyCount);

    player.x = SCREEN_WIDTH / 2.0f - player.width / 2.0f;
    player.y = SCREEN_HEIGHT - 60.0f;
    player.isAlive = true;
}

void ResetGameToLevel1(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets,
    Boss& boss, Bullet bossBullets[], int maxBossBullets)
{
    game.score = 0;
    game.level = 1;
    game.gameOver = false;
    game.gameWon = false;
    game.hitsToKill = 1;
    game.bossActive = false;

    InitPlayer(player);
    InitBullets(bullets, maxBullets);
    InitEnemiesForLevel(game, enemies, enemyCount);
    InitBoss(boss);
    InitBossBullets(bossBullets, maxBossBullets);
}

void SaveGame(const GameState& game, const Player& player, const Boss& boss)
{
    ofstream file("savegame.txt");
    if (!file.is_open()) {
        return;
    }

    file << game.score << ' '
        << game.level << ' '
        << game.highScore << ' '
        << game.hitsToKill << ' '
        << player.lives << ' '
        << game.bossActive << '\n';

    file.close();
}

void LoadGame(GameState& game, Player& player, Boss& boss)
{
    ifstream file("savegame.txt");
    if (!file.is_open()) {
        return;
    }

    int score, level, highScore, hitsToKill, lives;
    int bossActiveInt = 0;

    if (file >> score >> level >> highScore >> hitsToKill >> lives >> bossActiveInt) {
        game.score = score;
        game.level = level;
        game.highScore = highScore;
        game.hitsToKill = hitsToKill;
        player.lives = lives;
        game.bossActive = (bool)bossActiveInt;

        if (game.level < 1) game.level = 1;
        if (game.level > MAX_LEVEL + 1) game.level = MAX_LEVEL + 1;
        if (player.lives < 1) player.lives = 1;

        if (game.bossActive) {
            game.gameState = STATE_BOSS_FIGHT;
            InitBoss(boss);
        }
    }

    file.close();
}