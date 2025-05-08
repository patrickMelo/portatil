//
// Apps/DemoGame/DemoGame.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Assets.h"

// General --------------------------------------------------------------------

enum {
    typeIDPlayer,
    typeIDEnemy,
    typeIDEnemyProjectile,
    typeIDWave,
    typeIDExplosion,
    typeIDProjectile,
};

enum {
    layerBackground,
    layerPlayfield,
    layerEffects,
};

static uint backgroundColor = 0;

typedef void (*GameStateFunction)(const f16 speedMultiplier);

static GameStateFunction currentState = NULL;

static const SoundChannel ambientSoundChannel = SoundChannel4;

// Score and Health -----------------------------------------------------------

static int currentScore    = 0;
static int currentHealth   = 100;
static int enemyShotChance = 50;

static void dealDamage(const uint damageAmount);
static void increaseScore(const uint increaseAmount);
static void drawStats(void);

// Explosions -----------------------------------------------------------------

#define explosionFrameWidth  32
#define explosionFrameHeight 32
#define explosionFrames      14
#define explosionFPS         42

static const SoundChannel explosionChannel1 = SoundChannel2;
static const SoundChannel explosionChannel2 = SoundChannel3;

static int explosionSprite = -1;

static void initializeExplosions(void);
static void spawnExplosion(const f16 xPosition, const f16 yPosition);
static void updateExplosion(const int entityIndex);

// Waves ----------------------------------------------------------------------

#define numberOfWaves 8
#define waveSpeed     2

#define waveFrameWidth  22
#define waveFrameHeight 5
#define waveFrames      14
#define waveFPS         12

static int waveSprite = -1;

static void initializeWaves(void);
static void updateWave(const int entityIndex);

// Enemies --------------------------------------------------------------------

#define numberOfEnemies 5
#define enemyMinSpeed   1
#define enemyMaxSpeed   3

#define enemyFrameWidth  16
#define enemyFrameHeight 16
#define enemyFrames      3
#define enemyFPS         20

static int enemySprite = -1;

static void resetEnemy(const int entityIndex);
static void initializeEnemies(void);
static void updateEnemy(const int entityIndex);

// Projectiles ----------------------------------------------------------------

#define projectileSpeed 10

#define projectileFrameWidth  2
#define projectileFrameHeight 6
#define projectileFrames      12
#define projectileFPS         36

static const SoundChannel projectileChannel = SoundChannel1;

static int projectileSprite = -1;

static void initializeProjectiles(void);
static void updateProjectile(const int entityIndex);
static void updateEnemyProjectile(const int entityIndex);
static void spawnProjectile(const uint typeID, const f16 xPosition, const f16 yPosition);

// Player ---------------------------------------------------------------------

#define playerSpeed 5

#define playerFrameWidth  16
#define playerFrameHeight 16
#define playerFrames      3
#define playerFPS         20

static int playerSprite = -1;
static int playerEntity = -1;

// Game States ----------------------------------------------------------------

static void inGame(const f16 speedMultiplier);
static void gameOver(const f16 speedMultiplier);

// Implementation: Score and Health -------------------------------------------

static void dealDamage(const uint damageAmount) {
    currentHealth -= damageAmount;

    if (currentHealth <= 0) {
        currentHealth = 0;
    }
}

static void increaseScore(const uint increaseAmount) {
    currentScore += increaseAmount;
    enemyShotChance = 50 + (currentScore / 10);
}

static void drawStats(void) {
    SetTransparentColor(0);
    SetTextFont(&CustomFontImage);

    SetDrawAnchor(AnchorDefault);
    DrawText(1, 1, "Score:");
    DrawNumber(37, 1, currentScore);

    SetDrawAnchor(AnchorTop | AnchorRight);
    DrawText(ScreenWidth - 18, 1, "Health:");
    DrawNumber(ScreenWidth, 1, currentHealth);

    SetDrawAnchor(AnchorDefault);
    SetTextFont(NULL);
}

// Implementation: Explosions -------------------------------------------------

static void initializeExplosions(void) {
    explosionSprite = GetSprite(&ExplosionImage, 0, explosionFrameWidth, explosionFrameHeight);
    ConfigureSprite(explosionSprite, explosionFrames, explosionFPS);
}

static void spawnExplosion(const f16 xPosition, const f16 yPosition) {
    GetEntity(typeIDExplosion, explosionSprite, xPosition, yPosition);
    PlayTone(explosionChannel1, TriangleWave, 330, 200);
    PlayTone(explosionChannel2, SawtoothWave, 220, 300);
}

static void updateExplosion(const int entityIndex) {
    if (GetEntityFrameIndex(entityIndex) >= F16(explosionFrames - 1)) {
        ReleaseEntity(entityIndex);
    }
}

// Implementation: Waves ------------------------------------------------------

static void initializeWaves(void) {
    waveSprite = GetSprite(&WaveImage, 0, waveFrameWidth, waveFrameHeight);
    ConfigureSprite(waveSprite, waveFrames, waveFPS);

    SetActiveLayer(layerBackground);
    for (uint waveIndex = 0; waveIndex < numberOfWaves; waveIndex++) {
        int waveEntity = GetEntity(typeIDWave, waveSprite,
                                   F16(Random(0, ScreenWidth - waveFrameWidth)),
                                   F16(Random(0, ScreenHeight - waveFrameHeight)));

        SetEntitySpeed(waveEntity, 0, F16(waveSpeed));
        SetEntityDirection(waveEntity, 0, 1);
        SetEntityFrameIndex(waveEntity, F16(Random(0, waveFPS / 2)));
    }
}

static void updateWave(const int entityIndex) {
    if ((GetEntityPositionY(entityIndex) >= F16(ScreenHeight)) ||
        (GetEntityFrameIndex(entityIndex) >= F16(waveFrames - 1))) {
        SetEntityPosition(entityIndex,
                          F16(Random(0, ScreenWidth - waveFrameWidth)),
                          F16(Random(0, ScreenHeight - waveFrameHeight)));

        SetEntityFrameIndex(entityIndex, 0);
    }
}

// Implementation: Enemies ----------------------------------------------------

static void resetEnemy(const int entityIndex) {
    f16 xPosition = F16(Random(0, ScreenWidth - enemyFrameWidth));

    SetEntityPosition(entityIndex, xPosition, -F16(enemyFrameHeight - 1));

    SetEntitySpeed(entityIndex,
                   F16(Random(enemyMinSpeed, enemyMaxSpeed)),
                   F16(Random(enemyMinSpeed, enemyMaxSpeed)));

    SetEntityDirection(entityIndex, xPosition < F16(ScreenWidth / 2) ? 1 : -1, 1);
}

static void initializeEnemies(void) {
    enemySprite = GetSprite(&EnemyImage, 0, enemyFrameWidth, enemyFrameHeight);
    ConfigureSprite(enemySprite, enemyFrames, enemyFPS);

    SetActiveLayer(layerPlayfield);
    for (uint enemyIndex = 0; enemyIndex < numberOfEnemies; enemyIndex++) {
        resetEnemy(GetEntity(typeIDEnemy, enemySprite, 0, 0));
    }
}

static void updateEnemy(const int entityIndex) {
    if (!IsEntityOnScreen(entityIndex)) {
        resetEnemy(entityIndex);
        return;
    }

    if (GetEntityFrameIndex(entityIndex) == 0 && Random(0, 100 - enemyShotChance) == 0) {
        spawnProjectile(typeIDEnemyProjectile,
                        GetEntityPositionX(entityIndex) + F16(enemyFrameWidth / 2),
                        GetEntityPositionY(entityIndex) + F16(enemyFrameHeight));
    }
}

// Implementation: Projectiles ------------------------------------------------

static void initializeProjectiles(void) {
    projectileSprite = GetSprite(&ProjectileImage, 0, projectileFrameWidth, projectileFrameHeight);
    ConfigureSprite(projectileSprite, projectileFrames, projectileFPS);
}

static void updateProjectile(const int entityIndex) {
    f16 projectileY = GetEntityPositionY(entityIndex);

    if (projectileY <= 0) {
        ReleaseEntity(entityIndex);
        return;
    }

    int enemyIndex = GetCollidingEntityIndex(entityIndex, typeIDEnemy);

    if (enemyIndex >= 0) {
        f16 explosionX = GetEntityPositionX(enemyIndex) - F16((explosionFrameWidth - enemyFrameWidth) / 2);
        f16 explosionY = GetEntityPositionY(enemyIndex) - F16((explosionFrameHeight - enemyFrameHeight) / 2);

        SetActiveLayer(layerEffects);
        spawnExplosion(explosionX, explosionY);

        SetActiveLayer(layerPlayfield);
        ReleaseEntity(entityIndex);
        resetEnemy(enemyIndex);

        increaseScore(10);
    }
}

static void updateEnemyProjectile(const int entityIndex) {
    f16 projectileY = GetEntityPositionY(entityIndex);

    if (projectileY > F16(ScreenHeight)) {
        ReleaseEntity(entityIndex);
        return;
    }

    int playerIndex = GetCollidingEntityIndex(entityIndex, typeIDPlayer);

    if (playerIndex >= 0) {
        f16 explosionX = GetEntityPositionX(playerIndex) - F16((explosionFrameWidth - enemyFrameWidth) / 2);
        f16 explosionY = GetEntityPositionY(playerIndex) - F16(explosionFrameHeight / 2);

        SetActiveLayer(layerEffects);
        spawnExplosion(explosionX, explosionY);

        SetActiveLayer(layerPlayfield);
        ReleaseEntity(entityIndex);
        dealDamage(5);
    }
}

static void spawnProjectile(const uint typeID, const f16 xPosition, const f16 yPosition) {
    int projectileEntity = GetEntity(typeID, projectileSprite, xPosition, yPosition);

    SetEntitySpeed(projectileEntity, 0, F16(projectileSpeed));
    SetEntityDirection(projectileEntity, 0, typeID == typeIDProjectile ? -1 : 1);

    if (typeID == typeIDProjectile) {
        PlayTone(projectileChannel, SawtoothWave, 880, 100);
    }
}

// Implementation: Player -----------------------------------------------------

static void initializePlayer(void) {
    playerSprite = GetSprite(&PlaneImage, 0, playerFrameWidth, playerFrameHeight);
    ConfigureSprite(playerSprite, playerFrames, playerFPS);

    SetActiveLayer(layerPlayfield);
    playerEntity = GetEntity(typeIDPlayer, playerSprite, F16((ScreenWidth - playerFrameWidth) / 2), F16(ScreenHeight - playerFrameHeight - 2));
    SetEntitySpeed(playerEntity, F16(playerSpeed), 0);
}

static void updatePlayer(const int entityIndex) {
    SetEntityDirection(entityIndex, GetInputAxis(ButtonLeft, ButtonRight), 0);

    f16 playerPosition = GetEntityPositionX(entityIndex);

    if (playerPosition < F16(2)) {
        SetEntityPosition(entityIndex, F16(2), GetEntityPositionY(entityIndex));
    } else if (playerPosition > F16(ScreenWidth - 18)) {
        SetEntityPosition(entityIndex, F16(ScreenWidth - 18), GetEntityPositionY(entityIndex));
    }

    int enemyIndex = GetCollidingEntityIndex(entityIndex, typeIDEnemy);

    if (enemyIndex >= 0) {
        f16 explosionX = GetEntityPositionX(enemyIndex) - F16((explosionFrameWidth - enemyFrameWidth) / 2);
        f16 explosionY = GetEntityPositionY(playerEntity) - F16(explosionFrameHeight / 2);

        SetActiveLayer(layerEffects);
        spawnExplosion(explosionX, explosionY);

        SetActiveLayer(layerPlayfield);
        resetEnemy(enemyIndex);
        dealDamage(10);
    }
}

// Implementation: Game States ------------------------------------------------

static void updateEntities() {
    SetActiveLayer(layerBackground);

    for (uint entityIndex = 0; entityIndex < numberOfWaves; entityIndex++) {
        updateWave(entityIndex);
    }

    SetActiveLayer(layerPlayfield);
    uint numberOfEntities = GetNumberOfEntities();

    for (uint entityIndex = 0; entityIndex < numberOfEntities; entityIndex++) {
        switch (GetEntityTypeID(entityIndex)) {
            case typeIDPlayer: {
                if (currentHealth > 0) {
                    updatePlayer(entityIndex);
                }

                break;
            }

            case typeIDEnemy: {
                updateEnemy(entityIndex);
                break;
            }

            case typeIDProjectile: {
                updateProjectile(entityIndex);
                break;
            }

            case typeIDEnemyProjectile: {
                updateEnemyProjectile(entityIndex);
                break;
            }
        }
    }

    SetActiveLayer(layerEffects);
    numberOfEntities = GetNumberOfEntities();

    for (uint entityIndex = 0; entityIndex < numberOfEntities; entityIndex++) {
        updateExplosion(entityIndex);
    }
}

static void inGame(const f16 speedMultiplier) {
    SetActiveLayer(layerPlayfield);

    playerEntity = FindEntityIndex(typeIDPlayer, 1);

    if (IsButtonJustPressed(ButtonY)) {
        spawnProjectile(typeIDProjectile,
                        GetEntityPositionX(playerEntity) + F16(playerFrameWidth / 2),
                        GetEntityPositionY(playerEntity) - F16(projectileFrameHeight));
    }

    updateEntities();

    if (currentHealth == 0) {
        currentState = gameOver;
        StopChannel(ambientSoundChannel);
    }

    SyncEngine();
    drawStats();
}

static inline void newGame(void) {
    SetActiveLayer(layerPlayfield);

    for (uint enemyIndex = 0; enemyIndex < numberOfEnemies; enemyIndex++) {
        resetEnemy(FindEntityIndex(typeIDEnemy, enemyIndex + 1));
    }

    currentHealth   = 100;
    currentScore    = 0;
    currentState    = inGame;
    enemyShotChance = 50;

    PlayTone(ambientSoundChannel, SquareWave, 220, PlayForever);
}

static void gameOver(const f16 speedMultiplier) {
    DrawRectangle(20, 20, ScreenWidth - 40, 60, 0);
    DrawText((ScreenWidth - 54) / 2, 30, "Game Over");
    DrawText((ScreenWidth - 84) / 2, 50, "Press X Button");
    DrawText((ScreenWidth - 60) / 2, 60, "to Restart");

    if (IsButtonJustPressed(ButtonX)) {
        newGame();
    }
}

// Setup and Sync -------------------------------------------------------------

bool AppSetup(void) {
    initializeWaves();
    initializeEnemies();
    initializeProjectiles();
    initializeExplosions();
    initializePlayer();

    backgroundColor = GetColorIndex(0, 102, 230);
    SetChannelVolume(ambientSoundChannel, 10);
    SetChannelVolume(projectileChannel, 40);

    newGame();
    return true;
}

void AppSync(const f16 speedMultiplier) {
    ClearScreen(backgroundColor);
    currentState(speedMultiplier);
}