#include "SpaceShooter.h"
#include<raylib.h>
#include<iostream>
#include <fstream>
using namespace std;

int main()
{
    GameResources resources = { 0 };
    InitWindowAndResources(resources);
    PlayMusicStream(resources.gameTheme);
    SetMusicVolume(resources.gameTheme, 0.75f);
	
    GameState game;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Bullet bullets[MAX_BULLETS];
    int enemyCount = 0;

    InitGame(game, player, enemies, enemyCount, bullets, MAX_BULLETS);
    RunGameLoop(game, player, enemies, enemyCount, bullets, MAX_BULLETS, resources);

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


    // Unload Textures
    UnloadTexture(res.bulletTexture);
    UnloadTexture(res.enemyTexture);
    UnloadTexture(res.playerTexture);
    UnloadTexture(res.backgroundTexture);

    // Unload Sounds
    UnloadSound(res.shootSound);
    UnloadSound(res.explodeSound);
    UnloadSound(res.gameOverSound);
    UnloadSound(res.winSound);
    UnloadSound(res.playerHitSound);
    UnloadMusicStream(res.gameTheme);

    CloseAudioDevice(); // <--- Close audio system
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

void InitEnemiesForLevel(const GameState& game,
    Enemy enemies[], int& enemyCount)
{
    int level = game.level;

    // Number of enemies increases with level
    enemyCount = 3 + level * 3;
    if (enemyCount > MAX_ENEMIES) enemyCount = MAX_ENEMIES;

    float baseSpeed = 1.0f + level * 0.3f;

    for (int i = 0; i < enemyCount; i++) {
        enemies[i].width = 80;
        enemies[i].height = 80;

        // Spawn around the center of the screen with random x
        int centerX = SCREEN_WIDTH / 2;
        int halfRange = 200;

        int minX = centerX - halfRange;
        if (minX < 0) minX = 0;

        int maxX = centerX + halfRange - enemies[i].width;
        if (maxX > SCREEN_WIDTH - enemies[i].width) {
            maxX = SCREEN_WIDTH - enemies[i].width;
        }

        enemies[i].x = (float)GetRandomValue(minX, maxX);
        enemies[i].y = (float)GetRandomValue(60, 200);

        float randomOffset = (float)GetRandomValue(-3, 3) * 0.1f; // -0.3 .. +0.3
        enemies[i].speed = baseSpeed + randomOffset;
        if (enemies[i].speed < 0.5f) enemies[i].speed = 0.5f;

        enemies[i].health = game.hitsToKill;  // NEW: how many hits for this wave
        enemies[i].active = true;
    }

    // Any remaining enemies are inactive
    for (int i = enemyCount; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        enemies[i].health = 0;
    }
}


void InitGame(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets)
{
    // Default values
    game.score = 0;
    game.level = 1;
    game.highScore = 0;
    game.hitsToKill = 1;
    game.gameOver = false;
    game.gameWon = false;
    game.gameState = STATE_MENU;

    InitPlayer(player);
    player.lives = 3;  // default lives

    // Try to load saved game (if savegame.txt doesn't exist, LoadGame does nothing)
    LoadGame(game, player);

    // Now set up bullets and enemies for the (maybe updated) level
    InitBullets(bullets, maxBullets);
    InitEnemiesForLevel(game, enemies, enemyCount);
}



// ---------------------------------------------------------
// Main game loop
// ---------------------------------------------------------
// Add "const GameResources& res" here too
void RunGameLoop(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res)

{
    while (!WindowShouldClose())
    {
        UpdateMusicStream(res.gameTheme);
        if (IsKeyPressed(KEY_ESCAPE)) {
            SaveGame(game, player);
            break;
        }

        // Update
        if (game.gameState == STATE_MENU) {
            HandleStartScreenInput(game, player, enemies, enemyCount, bullets, maxBullets);
        }
        else if (game.gameState == STATE_PLAYING) {
            UpdateGame(game, player, enemies, enemyCount, bullets, maxBullets, res);
        }
        else if (game.gameState == STATE_GAME_OVER) {
            HandleGameOverInput(game, player, enemies, enemyCount, bullets, maxBullets, res);
        }
        else if (game.gameState == STATE_WIN) {
            HandleWinScreenInput(game, player, enemies, enemyCount, bullets, maxBullets, res);
        }


        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        if (game.gameState == STATE_MENU) {
            DrawStartScreen(game);
        }
        else if (game.gameState == STATE_PLAYING) {
            DrawGame(game, player, enemies, enemyCount, bullets, maxBullets, res);
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
    DrawText("- There are 10 levels. Clear them all to win.", 60, y, 20, LIGHTGRAY); y += 32;

    // Menu options
    DrawText("MENU", 60, y, 24, YELLOW);
    y += 30;
    DrawText("Press N     - New Game (start from Level 1)", 60, y, 20, GREEN); y += 24;
    DrawText("Press L     - Load Saved Game (if available)", 60, y, 20, GREEN); y += 24;
    DrawText("Press ENTER - Same as New Game", 60, y, 20, GREEN); y += 24;

    // High score display
    DrawText(TextFormat("Best Score (this run): %d", game.highScore),
        10, 10, 20, SKYBLUE);
}



void HandleStartScreenInput(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets)
{
    // ENTER behaves like New Game for convenience
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_N)) {
        // Start a completely new game from level 1
        ResetGameToLevel1(game, player, enemies, enemyCount, bullets, maxBullets);
        game.gameState = STATE_PLAYING;
    }
    else if (IsKeyPressed(KEY_L)) {
        // Try to load from file
        LoadGame(game, player);                 // restores score, level, hitsToKill, lives

        // Re-create bullets and enemies for the loaded level
        InitBullets(bullets, maxBullets);
        InitEnemiesForLevel(game, enemies, enemyCount);

        // Make sure basic flags are correct
        player.isAlive = true;
        game.gameOver = false;
        game.gameWon = false;

        game.gameState = STATE_PLAYING;
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
    Bullet bullets[], int maxBullets, const GameResources& res)
{
    if (IsKeyPressed(KEY_ENTER)) {
        ResetGameToLevel1(game, player, enemies, enemyCount, bullets, maxBullets);
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
    Bullet bullets[], int maxBullets, const GameResources& res)
{
    if (IsKeyPressed(KEY_ENTER)) {
        ResetGameToLevel1(game, player, enemies, enemyCount, bullets, maxBullets);
        game.gameState = STATE_PLAYING;
        PlayMusicStream(res.gameTheme);
    }
}

// ---------------------------------------------------------
// Game update & drawing (PLAYING state)
// ---------------------------------------------------------
// Change signature to accept GameResources
void UpdateGame(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res)
{
    UpdatePlayer(player);

    // Pass 'res' for shooting sound
    HandlePlayerShooting(player, bullets, maxBullets, res);

    UpdateBullets(bullets, maxBullets);
    UpdateEnemies(enemies, enemyCount);

    // Pass 'res' for explosion sounds
    CheckBulletEnemyCollisions(bullets, maxBullets, enemies, enemyCount, game, res);

    if (CheckEnemyPlayerCollisions(enemies, enemyCount, player)) {
        // Pass 'res' for game over sound
        HandlePlayerHit(game, player, enemies, enemyCount, bullets, maxBullets, res);
    }

    UpdateScoreAndLevel(game, player, enemies, enemyCount, bullets, maxBullets,res);
}

void DrawGame(const GameState& game, const Player& player,
    const Enemy enemies[], int enemyCount,
    const Bullet bullets[], int maxBullets, const GameResources& res) 

{
    if (res.backgroundTexture.id > 0) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)res.backgroundTexture.width, (float)res.backgroundTexture.height };
        Rectangle destRec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; // Fit to screen
        Vector2 origin = { 0, 0 };
        DrawTexturePro(res.backgroundTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    }
    if (player.isAlive) {

        Rectangle sourceRec = { 0.0f, 0.0f, (float)res.playerTexture.width, (float)res.playerTexture.height };
        Rectangle destRec = { player.x, player.y, player.width, player.height };
        Vector2 origin = { 0, 0 };
        DrawTexturePro(res.playerTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    }

    // Inside DrawGame function...

    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            // Use the texture from the "res" bucket
            if (res.enemyTexture.id > 0) {
                Rectangle source = { 0, 0, (float)res.enemyTexture.width, (float)res.enemyTexture.height };
                Rectangle dest = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
                Vector2 origin = { 0, 0 };

                DrawTexturePro(res.enemyTexture, source, dest, origin, 0.0f, WHITE);
            }
            
        }
    }

   
    // Draw Bullets
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {

            // Check if texture exists
            if (res.bulletTexture.id > 0) {
                Rectangle source = { 0, 0, (float)res.bulletTexture.width, (float)res.bulletTexture.height };

                // This scales the image to fit your bullet's width (5) and height (10)
                Rectangle dest = { bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height };

                Vector2 origin = { 0, 0 };
                DrawTexturePro(res.bulletTexture, source, dest, origin, 0.0f, WHITE);
            }
            else {
                // Fallback (Yellow Rectangle)
                DrawRectangle((int)bullets[i].x, (int)bullets[i].y, (int)bullets[i].width, (int)bullets[i].height, YELLOW);
            }
        }
    }
    

    DrawHUD(game, player);
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
    Bullet bullets[], int maxBullets, const GameResources& res) // Add 'res'
{
    if (IsKeyPressed(KEY_SPACE)) {
        for (int i = 0; i < maxBullets; i++) {
            if (!bullets[i].active) {
                // Play sound
                PlaySound(res.shootSound);

                bullets[i].active = true;
                bullets[i].x = player.x + player.width / 2 - bullets[i].width / 2;
                bullets[i].y = player.y - bullets[i].height;
                break;
            }
        }
    }
}

// ---------------------------------------------------------
// Bullets & Enemies
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
    GameState& game, const GameResources& res) // Add 'res'
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

                // Play hit/explosion sound
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

void HandlePlayerHit(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res)
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

    // Reset player
    player.isAlive = true;
    player.x = SCREEN_WIDTH / 2.0f - player.width / 2.0f;
    player.y = SCREEN_HEIGHT - 60.0f;

    // Clear bullets
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].active = false;
    }

    // Reset enemies for current level & current hitsToKill
    InitEnemiesForLevel(game, enemies, enemyCount);
}


// ---------------------------------------------------------
// HUD, scoring, level progression
// ---------------------------------------------------------
void DrawHUD(const GameState& game, const Player& player)
{
    DrawText(TextFormat("Score: %d", game.score),
        10, 10, 20, RAYWHITE);

    DrawText(TextFormat("Level: %d", game.level),
        10, 35, 20, RAYWHITE);

    DrawText(TextFormat("Lives: %d", player.lives),
        10, 60, 20, RAYWHITE);

    DrawText(TextFormat("Best: %d", game.highScore),
        SCREEN_WIDTH - 180, 10, 20, GREEN);
}


void UpdateScoreAndLevel(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets, const GameResources& res)
{
    bool allDead = AreAllEnemiesDestroyed(enemies, enemyCount);

    // 1) Check if we reached the score threshold for this level
    if (game.score >= game.level * 10) {
        game.level++;

        if (game.level > MAX_LEVEL) {
            game.gameWon = true;
            game.gameState = STATE_WIN;
            StopMusicStream(res.gameTheme);
            PlaySound(res.winSound);
        }
        else {
            // New level starts with easier enemies again
            game.hitsToKill = 1;
            ResetLevel(game, player, enemies, enemyCount, bullets, maxBullets);
        }
    }
    // 2) Not enough score, but all enemies are dead ? spawn another, harder wave
    else if (allDead) {
        game.hitsToKill++;
        InitEnemiesForLevel(game, enemies, enemyCount);
    }
}

void ResetLevel(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets)
{
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].active = false;
    }

    InitEnemiesForLevel(game, enemies, enemyCount);

    player.x = SCREEN_WIDTH / 2.0f - player.width / 2.0f;
    player.y = SCREEN_HEIGHT - 60.0f;
    player.isAlive = true;
}


void ResetGameToLevel1(GameState& game, Player& player,
    Enemy enemies[], int& enemyCount,
    Bullet bullets[], int maxBullets)
{
    game.score =0;
    game.level = 1;
    game.gameOver = false;
    game.gameWon = false;
    game.highScore ; 
    game.hitsToKill = 1; // NEW: reset enemy health for new game

    InitPlayer(player);
    InitBullets(bullets, maxBullets);
    InitEnemiesForLevel(game, enemies, enemyCount);
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

void SaveGame(const GameState& game, const Player& player)
{
    ofstream file("savegame.txt");
    if (!file.is_open()) {
        return;
    }

    // Save: score, level, highScore, hitsToKill, lives
    file << game.score << ' '
        << game.level << ' '
        << game.highScore << ' '
        << game.hitsToKill << ' '
        << player.lives << '\n';

    file.close();
}

void LoadGame(GameState& game, Player& player)
{
    ifstream file("savegame.txt");
    if (!file.is_open()) {
        // No save file yet
        return;
    }

    int score, level, highScore, hitsToKill, lives;
    if (file >> score >> level >> highScore >> hitsToKill >> lives) {
        game.score = score;
        game.level = level;
        game.highScore = highScore;
        game.hitsToKill = hitsToKill;
        player.lives = lives;

        if (game.level < 1)         game.level = 1;
        if (game.level > MAX_LEVEL) game.level = MAX_LEVEL;
        if (player.lives < 1)       player.lives = 1;
    }

    file.close();
}