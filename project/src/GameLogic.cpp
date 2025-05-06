#include "GameLogic.h"
#include "UI.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "Constants.h"

void generatePlatforms(std::vector<Platform>& platforms) {
    platforms.clear();            
    platforms.reserve(NUM_PLATFORMS); 
    for (int i = 0; i < NUM_PLATFORMS; ++i) {
      
        Platform p{ static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH)),
                   static_cast<float>(WINDOW_HEIGHT - (i * (WINDOW_HEIGHT / NUM_PLATFORMS))),
                   PLATFORM_WIDTH, PLATFORM_HEIGHT,
                   static_cast<PlatformType>(rand() % 3),
                   true,                         
                   (rand() % 2 == 0) ? 1.0f : -1.0f }; 

        p.sprite = (p.type == NORMAL) ? SPRITE_PLATFORM_NORMAL :
            (p.type == BREAKING) ? SPRITE_PLATFORM_BREAKING :
            SPRITE_PLATFORM_MOVING;

       
        p.powerUp.active = (static_cast<float>(rand()) / RAND_MAX) < POWERUP_SPAWN_CHANCE;
        if (p.powerUp.active) {
            PowerUpType type = (rand() % 2 == 0) ? SHIELD : SLASH_BOOST; 
            p.powerUp = { p.x + (p.w - POWERUP_SIZE) / 2.0f, 
                         p.y - POWERUP_SIZE, POWERUP_SIZE, POWERUP_SIZE,
                         true, type,
                         (type == SHIELD) ? SPRITE_SHIELD : SPRITE_SLASH_BOOST };
        }
        platforms.push_back(p);
    }
}


void trySpawnEnemy(std::vector<Enemy>& enemies, float spawnChance) {

    size_t activeEnemies = std::count_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.active; });
    if (activeEnemies >= MAX_ENEMIES) return;

    if (static_cast<float>(rand()) / RAND_MAX < spawnChance) {
        Enemy e{ static_cast<float>(rand() % (WINDOW_WIDTH - ENEMY_SIZE)),
                -ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE, true }; 
        e.centerX = e.x; e.centerY = e.y; 

        int type = rand() % 3;
        if (type == 0) {      
            e.speed = 1.0f + static_cast<float>(rand() % 20) / 10.0f;
            e.horizontalVelocity = (rand() % 2 == 0) ?
                ENEMY_HORIZONTAL_SPEED : -ENEMY_HORIZONTAL_SPEED;
            e.spriteFly[0] = SPRITE_ENEMY_FLY1; 
            e.spriteFly[1] = SPRITE_ENEMY_FLY2;
            e.spriteFly[2] = SPRITE_ENEMY_FLY3;
            e.spriteFly[3] = SPRITE_ENEMY_FLY4;
        }
        else if (type == 1) { 
            e.isShootingMonster = true;
            e.spriteShooting[0] = SPRITE_SHOOTING_MONSTER1;
            e.spriteShooting[1] = SPRITE_SHOOTING_MONSTER2;
            e.spriteShooting[2] = SPRITE_SHOOTING_MONSTER3;
            e.spriteShooting[3] = SPRITE_SHOOTING_MONSTER4;
            e.lastShotTime = SDL_GetTicks();
        }
        else {                
            e.isChasingEnemy = true;
            e.spriteChasing[0] = SPRITE_CHASING_ENEMY1;
            e.spriteChasing[1] = SPRITE_CHASING_ENEMY2;
            e.spriteChasing[2] = SPRITE_CHASING_ENEMY3;
            e.spriteChasing[3] = SPRITE_CHASING_ENEMY4;
        }
        enemies.push_back(e);
    }
}


void spawnPlayerOnMiddlePlatform(Player& player, const std::vector<Platform>& platforms) {
    float middleY = WINDOW_HEIGHT / 2.0f, closestDistance = -1; 
    const Platform* middlePlatform = nullptr;
    for (const auto& platform : platforms) {
        if (platform.active) {
            float distance = std::abs(platform.y - middleY);
            if (closestDistance == -1 || distance < closestDistance) {
                closestDistance = distance;
                middlePlatform = &platform;
            }
        }
    }
    if (middlePlatform) { 
        player.x = middlePlatform->x + (middlePlatform->w - player.w) / 2.0f;
        player.y = middlePlatform->y - player.h;
        player.vy = 0;
        player.jumpCount = 0;
    }
}


void performSlash(Player& player, Slash& slash, int direction, Mix_Chunk* slashSound) {
    Uint32 currentTime = SDL_GetTicks(); 
    int currentSlashDelay = player.slashBoostActive ? SLASH_BOOST_DELAY : SLASH_DELAY; 
    if (currentTime - player.lastSlashTime < currentSlashDelay) return;


    Slash newSlash{ 0, 0, 0, 0, true, direction, currentTime, 0, 0, SpriteRect{}, 255.0f };
    switch (direction) {
    case 1: 
        newSlash.x = player.x + player.w;
        newSlash.y = player.y + (player.h - SLASH_HEIGHT) / 2;
        newSlash.w = SLASH_WIDTH;
        newSlash.h = SLASH_HEIGHT;
        newSlash.vx = SLASH_SPEED;
        newSlash.sprite = SPRITE_SLASH_RIGHT;
        break;
    case 2: 
        newSlash.x = player.x - SLASH_WIDTH;
        newSlash.y = player.y + (player.h - SLASH_HEIGHT) / 2;
        newSlash.w = SLASH_WIDTH;
        newSlash.h = SLASH_HEIGHT;
        newSlash.vx = -SLASH_SPEED;
        newSlash.sprite = SPRITE_SLASH_LEFT;
        break;
    case 3: 
        newSlash.x = player.x + (player.w - SLASH_UP_WIDTH) / 2;
        newSlash.y = player.y - SLASH_UP_HEIGHT;
        newSlash.w = SLASH_UP_WIDTH;
        newSlash.h = SLASH_UP_HEIGHT;
        newSlash.vy = -SLASH_SPEED;
        newSlash.sprite = SPRITE_SLASH_UP;
        break;
    default: return;
    }
    slash = newSlash; 
    player.lastSlashTime = currentTime;
    player.slashDirection = direction;
    player.slashStartTime = currentTime;
    Mix_PlayChannel(-1, slashSound, 0); 
}


void shootMonsterBullet(Enemy& enemy, std::vector<MonsterBullet>& monsterBullets,
    const Player& player, Mix_Chunk* shootSound) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - enemy.lastShotTime < SHOOTING_MONSTER_SHOOT_DELAY) return; 
    float dx = player.x + player.w / 2.0f - (enemy.x + enemy.w / 2.0f);
    float dy = player.y + player.h / 2.0f - (enemy.y + enemy.h / 2.0f);
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance; dy /= distance; 
     
        monsterBullets.push_back({ enemy.x + enemy.w / 2.0f - MONSTER_BULLET_SIZE / 2.0f,
                                 enemy.y + enemy.h / 2.0f,
                                 dx * MONSTER_BULLET_SPEED, dy * MONSTER_BULLET_SPEED,
                                 true, false, SPRITE_MONSTER_BULLET });
        enemy.lastShotTime = currentTime;
        Mix_PlayChannel(-1, shootSound, 0); 
    }
}

void chasePlayer(Enemy& enemy, const Player& player) {
    float dx = player.x + player.w / 2.0f - (enemy.x + enemy.w / 2.0f);
    float dy = player.y + player.h / 2.0f - (enemy.y + enemy.h / 2.0f);
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance; dy /= distance; 
        enemy.x += dx * CHASING_ENEMY_SPEED; 
        enemy.y += dy * CHASING_ENEMY_SPEED; 
    }
}


void resetGame(Player& player, std::vector<Platform>& platforms, std::vector<Enemy>& enemies,
    std::vector<MonsterBullet>& monsterBullets, UI& ui, SDL_Renderer* renderer) {
    
    player = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 100.0f, 0.0f, 25, 40, 0,
              SPRITE_PLAYER_NEUTRAL, 0, 0, false, false, 0, 0, 0, nullptr, {0, 0, 0, 0} };
    if (player.slashBoostTimerTexture) SDL_DestroyTexture(player.slashBoostTimerTexture);
    player.slashBoostTimerTexture = nullptr;

    enemies.clear();         
    monsterBullets.clear();  
    generatePlatforms(platforms); 
    spawnPlayerOnMiddlePlatform(player, platforms); 
    ui.highScore = std::max(ui.highScore, ui.currentScore); 
    ui.currentScore = ui.totalEnemiesKilled = 0; 
    ui.totalScrollDistance = 0; 
    ui.startTime = SDL_GetTicks(); 
    ui.enemySpawnChance = 0.005f; 
    updateUITextures(renderer, ui, player); 
}