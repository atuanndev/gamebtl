#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 600;
const int MAP_WIDTH = 5000;
const int MAP_HEIGHT = 5000;
const int PLAYER_SIZE = 64;
const int BUG_SIZE = 20;
const float PLAYER_SPEED = 200.0f;
const float BUG_SPEED = 75.0f;
const float RANDOM_MOVE_CHANGE_TIME = 2.0f;
const float MOVE_TOWARDS_PLAYER_FACTOR = 0.3f;
const float BUG_SPAWN_INTERVAL = 2.0f;
const int BULLET_SPEED = 100.0f;
const int BULLET_SIZE = 10;
int HEALTH_POINT = 3;

struct camera {
    float x, y;
} camera = {MAP_WIDTH / 2 - SCREEN_WIDTH / 2, MAP_HEIGHT / 2 - SCREEN_HEIGHT / 2};

struct Animation {
    int frameCount;
    int currentFrame;
    float frameTime;
    float frameTimer;
    SDL_Rect* frames;
    int direction;
};

struct Entity {
    float x, y;
    float vx, vy;
    float timeToChangeDirection;
    float shootTimer;
    Animation anim;
};

struct Button {
    SDL_Rect rect;
    SDL_Texture* texture;
    std::string label;
    bool isHovered; // Thêm biến để theo dõi trạng thái hover
};

Button startButton;
Button quitButton;
SDL_Texture* highScoreTexture;

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};
GameState gameState = MENU;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
SDL_Texture* playerTexture = nullptr;
SDL_Texture* bugTexture = nullptr;
SDL_Texture* bulletTexture = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* menuTexture = nullptr;
Mix_Music* backgroundMusic = nullptr;
Mix_Chunk* killSound = nullptr;
Mix_Chunk* hitSound = nullptr;
Mix_Chunk* healthSound = nullptr;
bool running = true;
Entity player = {MAP_WIDTH / 2, MAP_HEIGHT / 2, 0, 0};
std::vector<Entity> bugs;
std::vector<Entity> bullets;
std::vector<int> highScores;
float bugSpawnTimer = 0;
int score = 0;
int HighScore = 0;

void resetGame() {
    player.x = MAP_WIDTH / 2;
    player.y = MAP_HEIGHT / 2;
    player.vx = 0;
    player.vy = 0;
    bugs.clear();
    bullets.clear();
    bugSpawnTimer = 0;
    score = 0;
    HEALTH_POINT = 3;
    gameState = PLAYING;
}

void spawnBugs() {
    int m = rand() % 5 + 3;
    int n = rand() % m + 1;
    for (int i = 0; i < n; i++) {
        float spawnRadius = SCREEN_WIDTH * 0.75f;
        float angle = (rand() % 360) * M_PI / 180.0f;
        float spawnX = camera.x + SCREEN_WIDTH / 2 + cos(angle) * spawnRadius;
        float spawnY = camera.y + SCREEN_HEIGHT / 2 + sin(angle) * spawnRadius;

        spawnX = std::max(0.0f, std::min(spawnX, (float)(MAP_WIDTH - BUG_SIZE)));
        spawnY = std::max(0.0f, std::min(spawnY, (float)(MAP_HEIGHT - BUG_SIZE)));
        float vx = cos(angle) * BUG_SPEED;
        float vy = sin(angle) * BUG_SPEED;
        float initialShootTimer = (rand() % 2000) / 1000.0f;
        bugs.push_back({spawnX, spawnY, vx, vy, RANDOM_MOVE_CHANGE_TIME, initialShootTimer});
    }
}

void handleInput() {
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    player.vx = player.vy = 0;

    if (keystates[SDL_SCANCODE_W]) player.vy = -PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_S]) player.vy = PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_A]) player.vx = -PLAYER_SPEED;
    if (keystates[SDL_SCANCODE_D]) player.vx = PLAYER_SPEED;
}

void initAnimation(Animation& anim, int frameCountPerDir, float frameTime, int frameWidth, int frameHeight) {
    anim.frameCount = frameCountPerDir;
    anim.currentFrame = 0;
    anim.frameTime = frameTime;
    anim.frameTimer = 0;
    anim.direction = 0;
    anim.frames = new SDL_Rect[frameCountPerDir * 4];
    for (int dir = 0; dir < 4; dir++) {
        for (int i = 0; i < frameCountPerDir; i++) {
            anim.frames[dir * frameCountPerDir + i] = {i * frameWidth, dir * frameHeight, frameWidth, frameHeight};
        }
    }
}

void updateAnimation(Animation& anim, float deltaTime, float vx, float vy) {
    bool isMoving = (vx != 0 || vy != 0);
    if (isMoving) {
        if (vy < 0) anim.direction = 1;
        else if (vy > 0) anim.direction = 0;
        else if (vx < 0) anim.direction = 3;
        else if (vx > 0) anim.direction = 2;

        anim.frameTimer += deltaTime;
        if (anim.frameTimer >= anim.frameTime) {
            anim.currentFrame = (anim.currentFrame + 1) % anim.frameCount;
            anim.frameTimer = 0;
        }
    } else {
        anim.currentFrame = 0;
    }
}

void shootBulletFromBug(Entity& bug) {
    float dx = player.x - bug.x;
    float dy = player.y - bug.y;
    float distance = sqrt(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance;
        dy /= distance;
    }

    float bulletVx = dx * BULLET_SPEED;
    float bulletVy = dy * BULLET_SPEED;
    bullets.push_back({bug.x, bug.y, bulletVx, bulletVy, 0, 0});
}

void updateBugs(float deltaTime) {
    for (auto& bug : bugs) {
        bug.timeToChangeDirection -= deltaTime;
        bug.shootTimer += deltaTime;

        if (bug.shootTimer >= 5.0f) {
            shootBulletFromBug(bug);
            bug.shootTimer = 0;
        }

        if (bug.timeToChangeDirection <= 0) {
            float angle = (rand() % 360) * M_PI / 180.0f;
            bug.vx = cos(angle) * BUG_SPEED;
            bug.vy = sin(angle) * BUG_SPEED;
            bug.timeToChangeDirection = RANDOM_MOVE_CHANGE_TIME;
        }

        float dx = player.x - bug.x;
        float dy = player.y - bug.y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance > 0) {
            dx /= distance;
            dy /= distance;
        }

        bug.vx = bug.vx * (1 - MOVE_TOWARDS_PLAYER_FACTOR) + dx * BUG_SPEED * MOVE_TOWARDS_PLAYER_FACTOR;
        bug.vy = bug.vy * (1 - MOVE_TOWARDS_PLAYER_FACTOR) + dy * BUG_SPEED * MOVE_TOWARDS_PLAYER_FACTOR;

        bug.x += bug.vx * deltaTime;
        bug.y += bug.vy * deltaTime;

        if (bug.x < 0 || bug.x > MAP_WIDTH - BUG_SIZE) bug.vx = -bug.vx;
        if (bug.y < 0 || bug.y > MAP_HEIGHT - BUG_SIZE) bug.vy = -bug.vy;
    }
}

void updatePlayer(float deltaTime) {
    player.x += player.vx * deltaTime;
    player.y += player.vy * deltaTime;

    if (player.x < 0) player.x = 0;
    if (player.y < 0) player.y = 0;
    if (player.x > MAP_WIDTH - PLAYER_SIZE) player.x = MAP_WIDTH - PLAYER_SIZE;
    if (player.y > MAP_HEIGHT - PLAYER_SIZE) player.y = MAP_HEIGHT - PLAYER_SIZE;

    camera.x = player.x - SCREEN_WIDTH / 2;
    camera.y = player.y - SCREEN_HEIGHT / 2;

    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > MAP_WIDTH - SCREEN_WIDTH) camera.x = MAP_WIDTH - SCREEN_WIDTH;
    if (camera.y > MAP_HEIGHT - SCREEN_HEIGHT) camera.y = MAP_HEIGHT - SCREEN_HEIGHT;

    updateAnimation(player.anim, deltaTime, player.vx, player.vy);
}

void removeBugsNearPlayer() {
    for (auto it = bugs.begin(); it != bugs.end();) {
        float dx = player.x - it->x;
        float dy = player.y - it->y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 60) {
            it = bugs.erase(it);
            score += 10;
            Mix_PlayChannel(-1, killSound, 0);
        } else {
            ++it;
        }
    }
}

void hight() {
    highScores.push_back(score);
    std::sort(highScores.begin(), highScores.end(), std::greater<int>());
    HighScore = highScores[0];
}

void bulletUpdate(float deltaTime) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->x += it->vx * deltaTime;
        it->y += it->vy * deltaTime;

        if (it->x < 0 || it->x > MAP_WIDTH || it->y < 0 || it->y > MAP_HEIGHT) {
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void checkCollision() {
    for (const auto& bug : bugs) {
        float dx = player.x - bug.x;
        float dy = player.y - bug.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < PLAYER_SIZE / 2 + BUG_SIZE / 2) {
            Mix_PlayChannel(-1, healthSound, 0);
            HEALTH_POINT--;
            if (HEALTH_POINT <= 0) {
                gameState = GAME_OVER;
            }
        }
    }
}

void checkBulletCollision() {
    for (auto it = bullets.begin(); it != bullets.end();) {
        float dx = player.x - it->x;
        float dy = player.y - it->y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < PLAYER_SIZE / 2 + BULLET_SIZE / 2) {
            it = bullets.erase(it);
            Mix_PlayChannel(-1, healthSound, 0);
            HEALTH_POINT--;
            if (HEALTH_POINT <= 0) {
                gameState = GAME_OVER;
            }
        } else {
            ++it;
        }
    }
}

void renderHealth() {
    Uint8 a = rand() % 255, b = rand() % 255, c = rand() % 255, d = rand() % 255;
    SDL_Color textColor = {a, b, c, d};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, ("Health: " + std::to_string(HEALTH_POINT)).c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {10, 50, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderScore() {
    Uint8 a = rand() % 255, b = rand() % 255, c = rand() % 255, d = rand() % 255;
    SDL_Color textColor = {a, b, c, d};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(score)).c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderGameOver() {

    SDL_Surface* menuSurface = IMG_Load("/project/data/images/menu.png");
    SDL_Texture* menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);

    SDL_Color textColor = {255, 0, 0, 255};
    SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, "Game Over", textColor);
    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    SDL_Rect gameOverRect = {SCREEN_WIDTH / 2 - gameOverSurface->w / 2, SCREEN_HEIGHT / 2 - gameOverSurface->h, gameOverSurface->w, gameOverSurface->h};
    SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);

    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(score)).c_str(), {255, 255, 255, 255});
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {SCREEN_WIDTH / 2 - scoreSurface->w / 2, SCREEN_HEIGHT / 2, scoreSurface->w, scoreSurface->h};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

    SDL_Surface* replaySurface = TTF_RenderText_Solid(font, "Press R to Play Again", {0, 255, 0, 255});
    SDL_Texture* replayTexture = SDL_CreateTextureFromSurface(renderer, replaySurface);
    SDL_Rect replayRect = {SCREEN_WIDTH / 2 - replaySurface->w / 2, SCREEN_HEIGHT / 2 + scoreSurface->h + 20, replaySurface->w, replaySurface->h};
    SDL_RenderCopy(renderer, replayTexture, NULL, &replayRect);

    SDL_Surface* quitSurface = TTF_RenderText_Solid(font, "Press Q to Quit", {0, 255, 0, 255});
    SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(renderer, quitSurface);
    SDL_Rect quitRect = {SCREEN_WIDTH / 2 - quitSurface->w / 2, SCREEN_HEIGHT / 2 + scoreSurface->h + 60, quitSurface->w, quitSurface->h};
    SDL_RenderCopy(renderer, quitTexture, NULL, &quitRect);

    SDL_Surface* highSurface = TTF_RenderText_Solid(font, ("High Score: " + std::to_string(HighScore)).c_str(), {255, 255, 255, 255});
    SDL_Texture* highTexture = SDL_CreateTextureFromSurface(renderer, highSurface);
    SDL_Rect hightRect = {SCREEN_WIDTH / 2 - highSurface->w / 2, SCREEN_HEIGHT / 2 + scoreSurface->h + 100, highSurface->w, highSurface->h};
    SDL_RenderCopy(renderer, highTexture, NULL, &hightRect);

    SDL_FreeSurface(gameOverSurface);
    SDL_FreeSurface(scoreSurface);
    SDL_FreeSurface(replaySurface);
    SDL_FreeSurface(quitSurface);
    SDL_FreeSurface(highSurface);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(replayTexture);
    SDL_DestroyTexture(quitTexture);
    SDL_DestroyTexture(highTexture);

    SDL_RenderPresent(renderer);
}

void renderMenu() {

    SDL_Rect backgroundRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, menuTexture, NULL, &backgroundRect);
    

    // Vẽ tiêu đề bằng font
    SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "GAME DINO SIGMA", {0, 255, 0, 255});
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_Rect titleRect = {SCREEN_WIDTH / 2 - titleSurface->w / 2, 100, titleSurface->w, titleSurface->h};
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

    // Vẽ nút Start với hiệu ứng thu nhỏ khi hover
    SDL_Rect startRect = startButton.rect;
    if (startButton.isHovered) {
        startRect.w = (int)(startButton.rect.w * 0.9); // Thu nhỏ 90%
        startRect.h = (int)(startButton.rect.h * 0.9);
        startRect.x += (startButton.rect.w - startRect.w) / 2; // Căn giữa
        startRect.y += (startButton.rect.h - startRect.h) / 2;
    }
    SDL_RenderCopy(renderer, startButton.texture, NULL, &startRect);

    // Vẽ nút Quit với hiệu ứng thu nhỏ khi hover
    SDL_Rect quitRect = quitButton.rect;
    if (quitButton.isHovered) {
        quitRect.w = (int)(quitButton.rect.w * 0.9); // Thu nhỏ 90%
        quitRect.h = (int)(quitButton.rect.h * 0.9);
        quitRect.x += (quitButton.rect.w - quitRect.w) / 2; // Căn giữa
        quitRect.y += (quitButton.rect.h - quitRect.h) / 2;
    }
    SDL_RenderCopy(renderer, quitButton.texture, NULL, &quitRect);

    // Vẽ High Score bằng font
    SDL_Surface* highScoreSurface = TTF_RenderText_Solid(font, ("Diem cao nhat: " + std::to_string(HighScore)).c_str(), {255, 255, 0, 255});
    highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
    SDL_Rect highScoreRect = {SCREEN_WIDTH / 2 - highScoreSurface->w / 2, SCREEN_HEIGHT / 2 + 150, highScoreSurface->w, highScoreSurface->h};
    SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);

    SDL_FreeSurface(titleSurface);
    SDL_FreeSurface(highScoreSurface);
    SDL_DestroyTexture(titleTexture);

    SDL_RenderPresent(renderer);
}

void render() {

    SDL_Rect backgroundRect = {0, 0, MAP_WIDTH, MAP_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);

    SDL_Rect playerRect = {(int)(player.x - camera.x), (int)(player.y - camera.y), PLAYER_SIZE, PLAYER_SIZE};
    int frameIndex = player.anim.direction * player.anim.frameCount + player.anim.currentFrame;
    SDL_RenderCopy(renderer, playerTexture, &player.anim.frames[frameIndex], &playerRect);

    for (const auto& bullet : bullets) {
        SDL_Rect bulletRect = {(int)(bullet.x - camera.x), (int)(bullet.y - camera.y), BULLET_SIZE, BULLET_SIZE};
        SDL_RenderCopy(renderer, bulletTexture, NULL, &bulletRect);
    }

    for (const auto& bug : bugs) {
        SDL_Rect bugRect = {(int)(bug.x - camera.x), (int)(bug.y - camera.y), BUG_SIZE, BUG_SIZE};
        SDL_RenderCopy(renderer, bugTexture, NULL, &bugRect);
    }

    renderScore();
    renderHealth();

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << std::endl;
        return -1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("GAME DINO SIGMA", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    font = TTF_OpenFont("/project/data/font/font.ttf", 50);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    backgroundMusic = Mix_LoadMUS("/project/data/audio/background.mp3");
    hitSound = Mix_LoadWAV("/project/data/audio/hit.wav");
    killSound = Mix_LoadWAV("/project/data/audio/kill.wav");
    healthSound = Mix_LoadWAV("/project/data/audio/health.wav");

    SDL_Surface* playerSurface = IMG_Load("/project/data/images/player.png");
    playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);

    SDL_Surface* menuSurface = IMG_Load("/project/data/images/menu.png");
    menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
    SDL_FreeSurface(menuSurface);

    SDL_Surface* bugSurface = IMG_Load("/project/data/images/bug.png");
    bugTexture = SDL_CreateTextureFromSurface(renderer, bugSurface);
    SDL_FreeSurface(bugSurface);

    SDL_Surface* bulletSurface = IMG_Load("/project/data/images/bullet.png");
    bulletTexture = SDL_CreateTextureFromSurface(renderer, bulletSurface);
    SDL_FreeSurface(bulletSurface);

    SDL_Surface* backgroundSurface = IMG_Load("/project/data/images/background.png");
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    // Tải hình ảnh cho nút Start
    SDL_Surface* startSurface = IMG_Load("/project/data/images/start.png");
    if (!startSurface) {
        std::cerr << "Unable to load start image! IMG_Error: " << IMG_GetError() << std::endl;
        return -1;
    }
    startButton.texture = SDL_CreateTextureFromSurface(renderer, startSurface);
    startButton.rect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 60, 200, 50};
    startButton.label = "Bat dau";
    startButton.isHovered = false; // Khởi tạo trạng thái hover
    SDL_FreeSurface(startSurface);

    // Tải hình ảnh cho nút Quit
    SDL_Surface* quitSurface = IMG_Load("/project/data/images/quit.png");
    if (!quitSurface) {
        std::cerr << "Unable to load quit image! IMG_Error: " << IMG_GetError() << std::endl;
        return -1;
    }
    quitButton.texture = SDL_CreateTextureFromSurface(renderer, quitSurface);
    quitButton.rect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 +20, 200, 50};
    quitButton.isHovered = false; // Khởi tạo trạng thái hover
    SDL_FreeSurface(quitSurface);

    initAnimation(player.anim, 4, 0.2f, PLAYER_SIZE, PLAYER_SIZE);
    srand(time(0));
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }

    Uint32 lastTick = SDL_GetTicks();

    while (running) {
        Uint32 currentTick = SDL_GetTicks();
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (gameState == MENU) {
                // Cập nhật trạng thái hover khi chuột di chuyển
                if (event.type == SDL_MOUSEMOTION) {
                    int mouseX = event.motion.x;
                    int mouseY = event.motion.y;

                    startButton.isHovered = (mouseX >= startButton.rect.x && mouseX <= startButton.rect.x + startButton.rect.w &&
                                             mouseY >= startButton.rect.y && mouseY <= startButton.rect.y + startButton.rect.h);

                    quitButton.isHovered = (mouseX >= quitButton.rect.x && mouseX <= quitButton.rect.x + quitButton.rect.w &&
                                            mouseY >= quitButton.rect.y && mouseY <= quitButton.rect.y + quitButton.rect.h);
                }

                // Xử lý click chuột
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    if (mouseX >= startButton.rect.x && mouseX <= startButton.rect.x + startButton.rect.w &&
                        mouseY >= startButton.rect.y && mouseY <= startButton.rect.y + startButton.rect.h) {
                        gameState = PLAYING;
                    }

                    if (mouseX >= quitButton.rect.x && mouseX <= quitButton.rect.x + quitButton.rect.w &&
                        mouseY >= quitButton.rect.y && mouseY <= quitButton.rect.y + quitButton.rect.h) {
                        running = false;
                    }
                }
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE && gameState == PLAYING) {
                    Mix_PlayChannel(-1, hitSound, 0);
                    removeBugsNearPlayer();
                }
                if (event.key.keysym.sym == SDLK_r && gameState == GAME_OVER) {
                    resetGame();
                }
                if (event.key.keysym.sym == SDLK_q && gameState == GAME_OVER) {
                    gameState= MENU;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE && gameState == PLAYING) {
                    gameState = MENU;
                }
            }
        }

        if (gameState == MENU) {
            renderMenu();
        } else if (gameState == PLAYING) {
            handleInput();
            updatePlayer(deltaTime);
            updateBugs(deltaTime);
            checkCollision();
            checkBulletCollision();
            bulletUpdate(deltaTime);
            hight();

            if ((bugSpawnTimer += deltaTime) >= BUG_SPAWN_INTERVAL) {
                spawnBugs();
                bugSpawnTimer = 0;
            }

            render();
        } else if (gameState == GAME_OVER) {
            renderGameOver();
        }

        SDL_Delay(16);
    }

    delete[] player.anim.frames;
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(killSound);
    Mix_FreeChunk(hitSound);
    Mix_FreeChunk(healthSound);
    Mix_CloseAudio();
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(bugTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(startButton.texture);
    SDL_DestroyTexture(quitButton.texture);
    SDL_DestroyTexture(highScoreTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}