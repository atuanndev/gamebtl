#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>
#include "Constants.h"
#include "Structures.h"
#include "UI.h"
#include "GameLogic.h"

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!initSDL(window, renderer)) return 1;

    SDL_Texture* atlasTexture = loadTexture(ATLAS_PATH.c_str(), renderer);
    Background background{ loadTexture(BACKGROUND_PATH.c_str(), renderer), 0, 0, 0.0f };
    if (!atlasTexture || !background.texture || SDL_QueryTexture(background.texture, nullptr, nullptr, &background.width, &background.height) < 0) {
        SDL_Log("Failed to load textures");
        return 1;
    }

    Mix_Chunk* jumpSound = Mix_LoadWAV("/project/assets/jump.wav");
    Mix_Chunk* slashSound = Mix_LoadWAV("/project/assets/slash.wav");
    Mix_Chunk* shootSound = Mix_LoadWAV("/project/assets/shoot.wav");
    Mix_Chunk* powerUpSound = Mix_LoadWAV("/project/assets/powerup.wav");
    Mix_Chunk* deathSound = Mix_LoadWAV("/project/assets/death.wav");
    Mix_Music* backgroundMusic = Mix_LoadMUS(MUSIC_PATH.c_str());
    if (!jumpSound || !slashSound || !shootSound || !powerUpSound || !deathSound || !backgroundMusic) {
        SDL_Log("Failed to load audio: %s", Mix_GetError());
        return 1;
    }
  
    UI ui{ TTF_OpenFont(FONT_PATH.c_str(), 24),      
          TTF_OpenFont(FONT_PATH.c_str(), 88),      
          nullptr, {0, 0, 0, 0},                    
          nullptr, {0, 0, 0, 0},                   
          nullptr, {0, 0, 0, 0},               
          nullptr, {0, 0, 0, 0},                 
          nullptr, {0, 0, 0, 0},                 
          nullptr, {0, 0, 0, 0},                
          nullptr, {0, 0, 0, 0},                 
          nullptr, {0, 0, 0, 0},                    
          0, 0, 0, 0, 0, 0.005f };              
    if (!ui.font || !ui.titleFont) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return 1;
    }

    srand(static_cast<unsigned>(time(nullptr)));
    Player player{ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 100.0f, 0.0f, 25, 40, 0,
                  SPRITE_PLAYER_NEUTRAL, 0, 0, false, false, 0, 0, 0, nullptr, {0, 0, 0, 0} };
    std::vector<Platform> platforms; 
    Slash slash{ 0, 0, 0, 0, false, 0, 0, 0, 0, SPRITE_SLASH_RIGHT, 255.0f }; 
    std::vector<Enemy> enemies;   
    std::vector<MonsterBullet> monsterBullets; 
    generatePlatforms(platforms);    
    spawnPlayerOnMiddlePlatform(player, platforms); 
    updateUITextures(renderer, ui, player);

    Mix_PlayMusic(backgroundMusic, -1); 

    bool quit = false;      
    GameState gameState = START_MENU; 
    SDL_Event e;                 
    float animationTime = 0.0f;   


    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN) {         
                switch (gameState) {
                case START_MENU:
                    if (e.key.keysym.sym == SDLK_SPACE) { 
                        gameState = PLAYING;
                        ui.startTime = SDL_GetTicks();
                    }
                    if (e.key.keysym.sym == SDLK_ESCAPE) quit = true;
                    break;
                case PLAYING:
                    switch (e.key.keysym.sym) {
                    case SDLK_SPACE: 
                        if (player.jumpCount < 1) {
                            player.vy = JUMP_VELOCITY;
                            player.jumpCount++;
                            Mix_PlayChannel(-1, jumpSound, 0);
                        }
                        break;
                    case SDLK_e: performSlash(player, slash, 1, slashSound); break; 
                    case SDLK_q: performSlash(player, slash, 2, slashSound); break; 
                    case SDLK_w: performSlash(player, slash, 3, slashSound); break; 
                    }
                    break;
                case GAME_OVER_MENU:
                    if (e.key.keysym.sym == SDLK_r) { 
                        resetGame(player, platforms, enemies, monsterBullets, ui, renderer);
                        gameState = PLAYING;
                    }
                    if (e.key.keysym.sym == SDLK_ESCAPE) quit = true; 
                    break;
                }
            }
        }

        Uint32 currentTime = SDL_GetTicks(); 

        if (gameState == PLAYING) {
            animationTime += 0.016f; 
            ui.enemySpawnChance = std::min(ui.enemySpawnChance + 0.00001f, 0.05f);


            ui.currentScore = static_cast<int>(ui.totalScrollDistance * SCORE_PER_SCROLL_UNIT) +
                ui.totalEnemiesKilled * SCORE_PER_ENEMY;
            updateUITextures(renderer, ui, player); 

     
            if (player.slashBoostActive && currentTime - player.slashBoostStartTime > SLASH_BOOST_DURATION) {
                player.slashBoostActive = false;
            }


            const Uint8* state = SDL_GetKeyboardState(nullptr);
            if (state[SDL_SCANCODE_LEFT]) {
                player.x -= PLAYER_SPEED;
                player.movementState = 2;
                player.sprite = SPRITE_PLAYER_LEFT;
            }
            else if (state[SDL_SCANCODE_RIGHT]) {
                player.x += PLAYER_SPEED;
                player.movementState = 1;
                player.sprite = SPRITE_PLAYER_RIGHT;
            }
            else {
                player.movementState = 0;
                player.sprite = SPRITE_PLAYER_NEUTRAL;
            }

            if (slash.active) {
                slash.x += slash.vx;
                slash.y += slash.vy;
                slash.alpha = 255.0f * (1.0f - static_cast<float>(currentTime - slash.startTime) / SLASH_DURATION);
                if (currentTime - slash.startTime > SLASH_DURATION) {
                    slash.active = false;
                    player.slashDirection = 0;
                }
            }

            player.x = (player.x + player.w < 0) ? WINDOW_WIDTH - player.w :
                (player.x > WINDOW_WIDTH) ? 0 : player.x;
            player.vy += GRAVITY; 
            player.y += player.vy; 

            background.y = static_cast<float>(fmod(background.y + 0.7f,
                static_cast<float>(background.height)));

            for (auto& platform : platforms) {
                if (platform.type == MOVING && platform.active) {
                    platform.x += MOVING_PLATFORM_SPEED * platform.direction;
                    if (platform.x <= 0 || platform.x + platform.w >= WINDOW_WIDTH)
                        platform.direction *= -1;
                    if (platform.powerUp.active)
                        platform.powerUp.x = platform.x + (platform.w - POWERUP_SIZE) / 2.0f;
                }
                if (checkCollision(player, platform)) { 
                    player.vy = JUMP_VELOCITY;
                    player.jumpCount = 0;
                    if (platform.type == BREAKING) platform.active = false; 
                    Mix_PlayChannel(-1, jumpSound, 0);
                }
                if (checkPlayerPowerUpCollision(player, platform.powerUp)) {
                    platform.powerUp.active = false;
                    Mix_PlayChannel(-1, powerUpSound, 0);
                    if (platform.powerUp.type == SHIELD) player.shieldActive = true;
                    else if (platform.powerUp.type == SLASH_BOOST) {
                        player.slashBoostActive = true;
                        player.slashBoostStartTime = currentTime;
                    }
                }
            }

            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                if (enemy.isShootingMonster)
                    shootMonsterBullet(enemy, monsterBullets, player, shootSound); 
                else if (enemy.isChasingEnemy)
                    chasePlayer(enemy, player); 
                else { 
                    enemy.centerX += enemy.horizontalVelocity;
                    enemy.x = std::max(0.0f, std::min(enemy.centerX,
                        static_cast<float>(WINDOW_WIDTH - enemy.w)));
                    if (enemy.x <= 0 || enemy.x + enemy.w >= WINDOW_WIDTH)
                        enemy.horizontalVelocity *= -1; 
                    enemy.y = enemy.centerY + sinf(animationTime * enemy.speed) * 50.0f; 
                }
            }

            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                if (checkPlayerEnemyCollision(player, enemy)) {
                    if (player.vy > 0) { 
                        enemy.active = false;
                        player.vy = JUMP_VELOCITY;
                        player.jumpCount = 0;
                        Mix_PlayChannel(-1, jumpSound, 0);
                        ++ui.totalEnemiesKilled;
                    }
                    else if (player.shieldActive) player.shieldActive = false;
                    else { 
                        Mix_PlayChannel(-1, deathSound, 0);
                        gameState = GAME_OVER_MENU;
                    }
                }
                if (slash.active && checkSlashEnemyCollision(slash, enemy)) {
                    enemy.active = false;
                    player.jumpCount = 0;
                    ++ui.totalEnemiesKilled;
                }
            }

            for (auto& bullet : monsterBullets) {
                if (!bullet.active) continue;
                bullet.x += bullet.vx;
                bullet.y += bullet.vy;
                if (bullet.y > WINDOW_HEIGHT || bullet.x < 0 || bullet.x > WINDOW_WIDTH)
                    bullet.active = false; 
                if (checkMonsterBulletPlayerCollision(bullet, player)) { 
                    bullet.active = false;
                    if (player.shieldActive) player.shieldActive = false;
                    else {
                        Mix_PlayChannel(-1, deathSound, 0);
                        gameState = GAME_OVER_MENU;
                    }
                }
                if (slash.active && checkSlashBulletCollision(slash, bullet)) { 
                    bullet.deflected = true;
                    bullet.vx = -bullet.vx;
                    bullet.vy = -bullet.vy;
                }
                for (auto& enemy : enemies) { 
                    if (checkBulletEnemyCollision(bullet, enemy)) {
                        bullet.active = false;
                        enemy.active = false;
                        ++ui.totalEnemiesKilled;
                    }
                }
            }

            if (player.y < WINDOW_HEIGHT / 2.5f) {
                float offset = (WINDOW_HEIGHT / 2.5f) - player.y; 
                player.y = WINDOW_HEIGHT / 2.5f; 
                ui.totalScrollDistance += offset;
                background.y += offset; 
                for (auto& platform : platforms) { 
                    platform.y += offset;
                    if (platform.powerUp.active) platform.powerUp.y += offset;
                    if (platform.y > WINDOW_HEIGHT) { 
                        platform = { static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH)),
                                   -PLATFORM_HEIGHT,
                                   PLATFORM_WIDTH, PLATFORM_HEIGHT,
                                   static_cast<PlatformType>(rand() % 3), true,
                                   (rand() % 2 == 0) ? 1.0f : -1.0f };
                        platform.sprite = (platform.type == NORMAL) ? SPRITE_PLATFORM_NORMAL :
                            (platform.type == BREAKING) ? SPRITE_PLATFORM_BREAKING :
                            SPRITE_PLATFORM_MOVING;
                        platform.powerUp.active = (static_cast<float>(rand()) / RAND_MAX) <
                            POWERUP_SPAWN_CHANCE;
                        if (platform.powerUp.active) {
                            PowerUpType type = (rand() % 2 == 0) ? SHIELD : SLASH_BOOST;
                            platform.powerUp = { platform.x + (platform.w - POWERUP_SIZE) / 2.0f,
                                               platform.y - POWERUP_SIZE, POWERUP_SIZE, POWERUP_SIZE,
                                               true, type,
                                               (type == SHIELD) ? SPRITE_SHIELD : SPRITE_SLASH_BOOST };
                        }
                    }
                }
                if (slash.active) slash.y += offset; 
                for (auto& enemy : enemies)
                    if (enemy.active) { enemy.centerY += offset; enemy.y += offset; } 
                for (auto& bullet : monsterBullets)
                    if (bullet.active) bullet.y += offset;
                trySpawnEnemy(enemies, ui.enemySpawnChance); 
            }

 
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                [](const Enemy& e) { return e.active && e.y > WINDOW_HEIGHT; }), enemies.end());
            if (player.y + player.h > WINDOW_HEIGHT) {
                Mix_PlayChannel(-1, deathSound, 0);
                gameState = GAME_OVER_MENU;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_Rect bgDst1 = { 0, static_cast<int>(background.y - background.height),
                          WINDOW_WIDTH, background.height };
        SDL_Rect bgDst2 = { 0, static_cast<int>(background.y),
                          WINDOW_WIDTH, background.height };
        SDL_RenderCopy(renderer, background.texture, nullptr, &bgDst1);
        SDL_RenderCopy(renderer, background.texture, nullptr, &bgDst2);

 
        SDL_Rect playerSrc, playerDst;
        SDL_Rect shieldSrc, shieldDst;
        SDL_Rect slashSrc, slashDst;

        switch (gameState) {
        case START_MENU:
            SDL_RenderCopy(renderer, ui.titleTexture, nullptr, &ui.titleRect); 
            SDL_RenderCopy(renderer, ui.playTexture, nullptr, &ui.playRect);   
            SDL_RenderCopy(renderer, ui.exitTexture, nullptr, &ui.exitRect);   
            SDL_RenderCopy(renderer, ui.instructionsTexture, nullptr,
                &ui.instructionsRect); 
            break;
        case PLAYING:
     
            playerSrc = player.sprite.toSDLRect();
            playerDst = { static_cast<int>(player.x), static_cast<int>(player.y),
                         player.w, player.h };
            SDL_RenderCopy(renderer, atlasTexture, &playerSrc, &playerDst);

       
            if (player.shieldActive) {
                shieldSrc = SPRITE_SHIELD_EQUIPPED.toSDLRect();
                shieldDst = { static_cast<int>(player.x - (SPRITE_SHIELD_EQUIPPED.w - player.w) / 2),
                             static_cast<int>(player.y - (SPRITE_SHIELD_EQUIPPED.h - player.h) / 2),
                             SPRITE_SHIELD_EQUIPPED.w, SPRITE_SHIELD_EQUIPPED.h };
                SDL_RenderCopy(renderer, atlasTexture, &shieldSrc, &shieldDst);
            }


            if (slash.active) {
                slashSrc = slash.sprite.toSDLRect();
                slashDst = { static_cast<int>(slash.x), static_cast<int>(slash.y),
                           slash.w, slash.h };
                SDL_SetTextureAlphaMod(atlasTexture, static_cast<Uint8>(slash.alpha));
                SDL_RenderCopy(renderer, atlasTexture, &slashSrc, &slashDst);
                SDL_SetTextureAlphaMod(atlasTexture, 255); 
            }

   
            for (const auto& platform : platforms) {
                if (platform.active) {
                    SDL_Rect src = platform.sprite.toSDLRect();
                    SDL_Rect dst = { static_cast<int>(platform.x), static_cast<int>(platform.y),
                                   platform.w, platform.h };
                    SDL_RenderCopy(renderer, atlasTexture, &src, &dst);
                    if (platform.powerUp.active) {
                        SDL_Rect powerUpSrc = platform.powerUp.sprite.toSDLRect();
                        SDL_Rect powerUpDst = { static_cast<int>(platform.powerUp.x),
                                              static_cast<int>(platform.powerUp.y),
                                              platform.powerUp.w, platform.powerUp.h };
                        SDL_RenderCopy(renderer, atlasTexture, &powerUpSrc, &powerUpDst);
                    }
                }
            }


            for (const auto& bullet : monsterBullets) {
                if (bullet.active) {
                    SDL_Rect src = bullet.sprite.toSDLRect();
                    SDL_Rect dst = { static_cast<int>(bullet.x), static_cast<int>(bullet.y),
                                   MONSTER_BULLET_SIZE, MONSTER_BULLET_SIZE };
                    SDL_RenderCopy(renderer, atlasTexture, &src, &dst);
                }
            }

      
            for (const auto& enemy : enemies) {
                if (enemy.active) {
                    SDL_Rect src = (enemy.isShootingMonster) ?
                        enemy.spriteShooting[static_cast<int>(animationTime *
                            ENEMY_FLY_ANIMATION_SPEED) % 4].toSDLRect() :
                        (enemy.isChasingEnemy) ?
                        enemy.spriteChasing[static_cast<int>(animationTime *
                            ENEMY_FLY_ANIMATION_SPEED) % 4].toSDLRect() :
                        enemy.spriteFly[static_cast<int>(animationTime *
                            ENEMY_FLY_ANIMATION_SPEED) % 4].toSDLRect();
                    SDL_Rect dst = { static_cast<int>(enemy.x), static_cast<int>(enemy.y),
                                   enemy.w, enemy.h };
                    SDL_RenderCopy(renderer, atlasTexture, &src, &dst);
                }
            }

      
            SDL_RenderCopy(renderer, ui.scoreTexture, nullptr, &ui.scoreRect);
            SDL_RenderCopy(renderer, ui.highScoreTexture, nullptr, &ui.highScoreRect);
            if (player.slashBoostActive && player.slashBoostTimerTexture) {
                SDL_RenderCopy(renderer, player.slashBoostTimerTexture, nullptr,
                    &player.slashBoostTimerRect); 
            }
            break;
        case GAME_OVER_MENU:
            SDL_RenderCopy(renderer, ui.gameOverTexture, nullptr, &ui.gameOverRect); 
            SDL_RenderCopy(renderer, ui.scoreTexture, nullptr, &ui.scoreRect);       
            SDL_RenderCopy(renderer, ui.highScoreTexture, nullptr, &ui.highScoreRect); 
            SDL_RenderCopy(renderer, ui.playAgainTexture, nullptr, &ui.playAgainRect); 
            break;
        }

        SDL_RenderPresent(renderer); 
        SDL_Delay(16); 
    }

    SDL_DestroyTexture(atlasTexture);
    SDL_DestroyTexture(background.texture);
    SDL_DestroyTexture(ui.scoreTexture);
    SDL_DestroyTexture(ui.highScoreTexture);
    SDL_DestroyTexture(ui.titleTexture);
    SDL_DestroyTexture(ui.playTexture);
    SDL_DestroyTexture(ui.exitTexture);
    SDL_DestroyTexture(ui.gameOverTexture);
    SDL_DestroyTexture(ui.playAgainTexture);
    SDL_DestroyTexture(ui.instructionsTexture);
    SDL_DestroyTexture(player.slashBoostTimerTexture);
    TTF_CloseFont(ui.font);
    TTF_CloseFont(ui.titleFont);
    Mix_FreeChunk(jumpSound);
    Mix_FreeChunk(slashSound);
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(powerUpSound);
    Mix_FreeChunk(deathSound);
    Mix_FreeMusic(backgroundMusic);
    Mix_Quit();          
    TTF_Quit();          
    IMG_Quit();         
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();         

    return 0; 
}