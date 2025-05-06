#pragma once
#include <vector>
#include "Structures.h"
#include "Constants.h"
#include <SDL2/SDL_mixer.h> 

void generatePlatforms(std::vector<Platform>& platforms);
void trySpawnEnemy(std::vector<Enemy>& enemies, float spawnChance);

inline bool checkCollision(const Player& player, const Platform& platform) {
    return platform.active &&
        player.x + player.w > platform.x &&
        player.x < platform.x + platform.w &&
        player.y + player.h > platform.y &&
        player.y + player.h < platform.y + platform.h + 5 && 
        player.vy > 0; 
}

inline bool checkPlayerEnemyCollision(const Player& player, const Enemy& enemy) {
    return enemy.active &&
        player.x + player.w > enemy.x &&
        player.x < enemy.x + enemy.w &&
        player.y + player.h > enemy.y &&
        player.y < enemy.y + enemy.h;
}

inline bool checkSlashEnemyCollision(const Slash& slash, const Enemy& enemy) {
    return slash.active && enemy.active &&
        slash.x + slash.w > enemy.x &&
        slash.x < enemy.x + enemy.w &&
        slash.y + slash.h > enemy.y &&
        slash.y < enemy.y + enemy.h;
}

inline bool checkMonsterBulletPlayerCollision(const MonsterBullet& bullet, const Player& player) {
    return bullet.active && !bullet.deflected &&
        bullet.x + MONSTER_BULLET_SIZE > player.x &&
        bullet.x < player.x + player.w &&
        bullet.y + MONSTER_BULLET_SIZE > player.y &&
        bullet.y < player.y + player.h;
}

inline bool checkSlashBulletCollision(const Slash& slash, const MonsterBullet& bullet) {
    return slash.active && bullet.active && !bullet.deflected &&
        slash.x + slash.w > bullet.x &&
        slash.x < bullet.x + MONSTER_BULLET_SIZE &&
        slash.y + slash.h > bullet.y &&
        slash.y < bullet.y + MONSTER_BULLET_SIZE;
}

inline bool checkBulletEnemyCollision(const MonsterBullet& bullet, const Enemy& enemy) {
    return bullet.active && bullet.deflected && enemy.active &&
        bullet.x + MONSTER_BULLET_SIZE > enemy.x &&
        bullet.x < enemy.x + enemy.w &&
        bullet.y + MONSTER_BULLET_SIZE > enemy.y &&
        bullet.y < enemy.y + enemy.h;
}

inline bool checkPlayerPowerUpCollision(const Player& player, const PowerUp& powerUp) {
    return powerUp.active &&
        player.x + player.w > powerUp.x &&
        player.x < powerUp.x + powerUp.w &&
        player.y + player.h > powerUp.y &&
        player.y < powerUp.y + powerUp.h;
}

void spawnPlayerOnMiddlePlatform(Player& player, const std::vector<Platform>& platforms);
void performSlash(Player& player, Slash& slash, int direction, Mix_Chunk* slashSound);
void shootMonsterBullet(Enemy& enemy, std::vector<MonsterBullet>& monsterBullets, const Player& player, Mix_Chunk* shootSound);
void chasePlayer(Enemy& enemy, const Player& player);
void resetGame(Player& player, std::vector<Platform>& platforms, std::vector<Enemy>& enemies,
    std::vector<MonsterBullet>& monsterBullets, UI& ui, SDL_Renderer* renderer);