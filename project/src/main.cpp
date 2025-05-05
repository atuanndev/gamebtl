#include <SDL2/SDL.h>         // Thư viện chính của SDL cho quản lý cửa sổ và sự kiện
#include <SDL2/SDL_image.h>   // Thư viện để tải ảnh (PNG, JPG, v.v.)
#include <SDL2/SDL_mixer.h>   // Thư viện để xử lý âm thanh và nhạc
#include <SDL2/SDL_ttf.h>     // Thư viện để render văn bản
#include <vector>        // Thư viện STL cho mảng động
#include <cstdlib>       // Thư viện cho hàm rand() và srand()
#include <ctime>         // Thư viện để lấy thời gian hệ thống
#include <cmath>         // Thư viện cho các hàm toán học (sin, sqrt, v.v.)
#include <string>        // Thư viện cho chuỗi ký tự
#include <iostream>      // Thư viện cho nhập/xuất console (dùng để debug)
#include <algorithm>     // Thư viện cho các thuật toán STL (count_if, remove_if)

// Hằng số cấu hình game
const int WINDOW_WIDTH = 650;        // Chiều rộng cửa sổ game
const int WINDOW_HEIGHT = 700;       // Chiều cao cửa sổ game
const float GRAVITY = 0.6f;          // Gia tốc trọng lực tác động lên người chơi
const float JUMP_VELOCITY = -12.0f;  // Vận tốc ban đầu khi nhảy (âm vì nhảy lên)
const float PLAYER_SPEED = 5.0f;     // Tốc độ di chuyển ngang của người chơi
const int PLATFORM_WIDTH = 60;       // Chiều rộng bệ đỡ
const int PLATFORM_HEIGHT = 12;      // Chiều cao bệ đỡ
const int NUM_PLATFORMS = 10;        // Số lượng bệ đỡ tối đa trên màn hình
const float MOVING_PLATFORM_SPEED = 2.0f; // Tốc độ di chuyển của bệ đỡ di động
const int SLASH_WIDTH = 60;          // Chiều rộng đòn chém ngang
const int SLASH_HEIGHT = 100;        // Chiều cao đòn chém ngang
const int SLASH_UP_WIDTH = 80;       // Chiều rộng đòn chém lên
const int SLASH_UP_HEIGHT = 50;      // Chiều cao đòn chém lên
const int SLASH_DURATION = 200;      // Thời gian tồn tại của đòn chém (ms)
const int SLASH_DELAY = 500;         // Thời gian chờ giữa các đòn chém (ms)
const int SLASH_BOOST_DELAY = 250;   // Thời gian chờ khi có boost (ms)
const float SLASH_SPEED = 5.0f;      // Tốc độ di chuyển của đòn chém
const int SLASH_BOOST_DURATION = 10000; // Thời gian hiệu lực của boost chém (ms)
const size_t MAX_ENEMIES = 8;        // Số lượng quái tối đa trên màn hình
const int ENEMY_SIZE = 30;           // Kích thước quái (rộng = cao)
const float ENEMY_HORIZONTAL_SPEED = 2.0f; // Tốc độ di chuyển ngang của quái bay
const float ENEMY_FLY_ANIMATION_SPEED = 4.0f; // Tốc độ chuyển frame animation của quái bay
const float CHASING_ENEMY_SPEED = 3.0f; // Tốc độ đuổi theo của quái đuổi
const float SHOOTING_MONSTER_SHOOT_DELAY = 2000.0f; // Thời gian chờ giữa các lần bắn của quái bắn
const float MONSTER_BULLET_SPEED = 7.0f; // Tốc độ đạn của quái bắn
const int MONSTER_BULLET_SIZE = 5;   // Kích thước đạn quái
const int POWERUP_SIZE = 20;         // Kích thước vật phẩm tăng sức mạnh
const float POWERUP_SPAWN_CHANCE = 0.02f; // Xác suất xuất hiện vật phẩm
const int SCORE_PER_SCROLL_UNIT = 2; // Điểm nhận được mỗi đơn vị cuộn màn hình
const int SCORE_PER_ENEMY = 500;     // Điểm nhận được khi tiêu diệt một quái
const std::string BACKGROUND_PATH = "/project/data/images/background.png"; // Đường dẫn file nền
const std::string MUSIC_PATH = "/project/data/audio/game.mp3"; // Đường dẫn file nhạc nền
const std::string FONT_PATH = "/project/data/font/font.ttf"; // Đường dẫn file font
const std::string ATLAS_PATH = "/project/data/images/player.png"; // Đường dẫn file atlas sprite

// Định nghĩa tọa độ sprite trong atlas
struct SpriteRect {
    int x, y, w, h;                  // Tọa độ x, y, chiều rộng, chiều cao trong atlas
    SDL_Rect toSDLRect() const { return { x, y, w, h }; } // Chuyển thành SDL_Rect
};

// Tọa độ các sprite cụ thể trong atlas
const SpriteRect SPRITE_PLAYER_NEUTRAL = { 61, 43, 19, 29 };
const SpriteRect SPRITE_PLAYER_RIGHT = { 118, 13, 20, 27 };
const SpriteRect SPRITE_PLAYER_LEFT = { 63, 16, 20, 27 };
const SpriteRect SPRITE_MONSTER_BULLET = { 17, 47, 5, 5 };
const SpriteRect SPRITE_ENEMY_FLY1 = { 32, 16, 16, 11 };
const SpriteRect SPRITE_ENEMY_FLY2 = { 48, 18, 16, 10 };
const SpriteRect SPRITE_ENEMY_FLY3 = { 32, 33, 16, 11 };
const SpriteRect SPRITE_ENEMY_FLY4 = { 48, 18, 16, 10 };
const SpriteRect SPRITE_SHOOTING_MONSTER1 = { 193, 1, 16, 14 };
const SpriteRect SPRITE_SHOOTING_MONSTER2 = { 209, 1, 16, 14 };
const SpriteRect SPRITE_SHOOTING_MONSTER3 = { 225, 1, 16, 14 };
const SpriteRect SPRITE_SHOOTING_MONSTER4 = { 1, 62, 16, 16 };
const SpriteRect SPRITE_CHASING_ENEMY1 = { 1, 16, 16, 11 };
const SpriteRect SPRITE_CHASING_ENEMY2 = { 18, 16, 14, 10 };
const SpriteRect SPRITE_CHASING_ENEMY3 = { 1, 32, 16, 11 };
const SpriteRect SPRITE_CHASING_ENEMY4 = { 18, 16, 14, 10 };
const SpriteRect SPRITE_SLASH_RIGHT = { 36, 43, 25, 30 };
const SpriteRect SPRITE_SLASH_LEFT = { 271, 1, 25, 30 };
const SpriteRect SPRITE_SLASH_UP = { 242, 1, 28, 24 };
const SpriteRect SPRITE_SHIELD = { 1, 46, 16, 16 };
const SpriteRect SPRITE_SHIELD_EQUIPPED = { 88, 14, 27, 30 };
const SpriteRect SPRITE_SLASH_BOOST = { 2, 81, 15, 14 };
const SpriteRect SPRITE_PLATFORM_NORMAL = { 145, 1, 48, 14 };
const SpriteRect SPRITE_PLATFORM_BREAKING = { 1, 1, 87, 14 };
const SpriteRect SPRITE_PLATFORM_MOVING = { 85, 1, 60, 12 };
// ... (các sprite khác)

// Các trạng thái game
enum GameState { START_MENU, PLAYING, GAME_OVER_MENU }; // Menu bắt đầu, đang chơi, menu kết thúc

// Cấu trúc người chơi
struct Player {
    float x, y, vy;                 // Vị trí x, y và vận tốc dọc
    int w, h;                       // Chiều rộng, chiều cao
    Uint32 lastSlashTime;           // Thời điểm thực hiện đòn chém cuối cùng
    SpriteRect sprite;              // Sprite hiện tại của người chơi
    int slashDirection;             // Hướng đòn chém (1: phải, 2: trái, 3: lên)
    Uint32 slashStartTime;          // Thời điểm bắt đầu đòn chém
    bool shieldActive;              // Trạng thái khiên bảo vệ
    bool slashBoostActive;          // Trạng thái boost đòn chém
    Uint32 slashBoostStartTime;     // Thời điểm bắt đầu boost chém
    int jumpCount;                  // Số lần nhảy còn lại
    int movementState;              // Trạng thái di chuyển (0: đứng, 1: phải, 2: trái)
    SDL_Texture* slashBoostTimerTexture; // Texture hiển thị thời gian boost còn lại
    SDL_Rect slashBoostTimerRect;   // Vị trí và kích thước hiển thị timer boost
};

// Cấu trúc đòn chém
struct Slash {
    float x, y;                     // Vị trí
    int w, h;                       // Kích thước
    bool active;                    // Trạng thái hoạt động
    int direction;                  // Hướng chém
    Uint32 startTime;               // Thời điểm bắt đầu
    float vx, vy;                   // Vận tốc ngang, dọc
    SpriteRect sprite;              // Sprite của đòn chém
    float alpha;                    // Độ trong suốt
};

// Cấu trúc đạn quái
struct MonsterBullet {
    float x, y, vx, vy;             // Vị trí và vận tốc
    bool active;                    // Trạng thái hoạt động
    bool deflected;                 // Trạng thái bị phản đòn
    SpriteRect sprite;              // Sprite của đạn
};

// Cấu trúc quái vật
struct Enemy {
    float x, y;                     // Vị trí
    int w, h;                       // Kích thước
    bool active;                    // Trạng thái hoạt động
    float centerX, centerY;         // Tọa độ trung tâm (dùng cho chuyển động sóng)
    float speed;                    // Tốc độ chuyển động
    float horizontalVelocity;       // Vận tốc ngang
    SpriteRect spriteFly[4];        // Các frame animation cho quái bay
    SpriteRect spriteShooting[4];   // Các frame animation cho quái bắn
    SpriteRect spriteChasing[4];    // Các frame animation cho quái đuổi
    bool isShootingMonster;         // Là quái bắn
    bool isChasingEnemy;            // Là quái đuổi
    Uint32 lastShotTime;            // Thời điểm bắn cuối cùng
};

// Loại vật phẩm tăng sức mạnh
enum PowerUpType { SHIELD, SLASH_BOOST, NONE }; // Khiên, boost chém, không có

// Cấu trúc vật phẩm
struct PowerUp {
    float x, y;                     // Vị trí
    int w, h;                       // Kích thước
    bool active;                    // Trạng thái hoạt động
    PowerUpType type;               // Loại vật phẩm
    SpriteRect sprite;              // Sprite của vật phẩm
};

// Loại bệ đỡ
enum PlatformType { NORMAL, BREAKING, MOVING }; // Thường, vỡ, di động

// Cấu trúc bệ đỡ
struct Platform {
    float x, y;                     // Vị trí
    int w, h;                       // Kích thước
    PlatformType type;              // Loại bệ đỡ
    bool active;                    // Trạng thái hoạt động
    float direction;                // Hướng di chuyển (cho bệ di động)
    SpriteRect sprite;              // Sprite của bệ đỡ
    PowerUp powerUp;                // Vật phẩm trên bệ đỡ
};

// Cấu trúc nền
struct Background {
    SDL_Texture* texture;           // Texture của nền
    int width, height;              // Kích thước nền
    float y;                        // Vị trí y (dùng để cuộn)
};

// Cấu trúc giao diện người dùng
struct UI {
    TTF_Font* font;                 // Font chữ nhỏ cho văn bản thông thường
    TTF_Font* titleFont;            // Font chữ lớn cho tiêu đề
    SDL_Texture* scoreTexture;      // Texture hiển thị điểm số
    SDL_Rect scoreRect;             // Vị trí và kích thước điểm số
    SDL_Texture* highScoreTexture;  // Texture hiển thị điểm cao nhất
    SDL_Rect highScoreRect;         // Vị trí và kích thước điểm cao nhất
    SDL_Texture* titleTexture;      // Texture tiêu đề game
    SDL_Rect titleRect;             // Vị trí và kích thước tiêu đề
    SDL_Texture* playTexture;       // Texture nút "Chơi"
    SDL_Rect playRect;              // Vị trí và kích thước nút "Chơi"
    SDL_Texture* exitTexture;       // Texture nút "Thoát"
    SDL_Rect exitRect;              // Vị trí và kích thước nút "Thoát"
    SDL_Texture* gameOverTexture;   // Texture "Bạn đã chết"
    SDL_Rect gameOverRect;          // Vị trí và kích thước "Bạn đã chết"
    SDL_Texture* playAgainTexture;  // Texture "Chơi lại"
    SDL_Rect playAgainRect;         // Vị trí và kích thước "Chơi lại"
    SDL_Texture* instructionsTexture; // Texture hướng dẫn chơi
    SDL_Rect instructionsRect;      // Vị trí và kích thước hướng dẫn
    int currentScore;               // Điểm số hiện tại
    int highScore;                  // Điểm cao nhất
    int totalEnemiesKilled;         // Tổng số quái đã tiêu diệt
    float totalScrollDistance;      // Tổng khoảng cách cuộn màn hình
    Uint32 startTime;               // Thời điểm bắt đầu chơi
    float enemySpawnChance;         // Xác suất xuất hiện quái
};

// Hàm tải texture từ file
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);      // Tải ảnh từ đường dẫn
    if (!surface) {                             // Kiểm tra lỗi
        SDL_Log("Failed to load: %s, %s", path, IMG_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // Chuyển surface thành texture
    SDL_FreeSurface(surface);                   // Giải phóng bộ nhớ surface
    return texture;
}

// Hàm tạo texture từ văn bản
SDL_Texture* createTextTexture(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, const SDL_Color& color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color); // Render văn bản thành surface
    if (!surface) return nullptr;               // Kiểm tra lỗi
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);  // Chuyển thành texture
    SDL_FreeSurface(surface);                   // Giải phóng surface
    return texture;
}

// Hàm cập nhật các texture giao diện
void updateUITextures(SDL_Renderer* renderer, UI& ui, Player& player) {
    static const SDL_Color white = { 255, 255, 255, 255 };    // Màu trắng cho văn Telegram: https://t.me/xAI_grok3 văn bản
    static const SDL_Color yellow = { 255, 255, 0, 255 };     // Màu vàng cho tiêu đề

    // Cập nhật texture điểm số
    if (ui.scoreTexture) SDL_DestroyTexture(ui.scoreTexture);  // Xóa texture cũ nếu có
    std::string scoreText = "Score: " + std::to_string(ui.currentScore) +
        "  Kills: " + std::to_string(ui.totalEnemiesKilled); // Tạo chuỗi điểm số
    ui.scoreTexture = createTextTexture(renderer, ui.font, scoreText, white); // Tạo texture mới
    SDL_QueryTexture(ui.scoreTexture, nullptr, nullptr, &ui.scoreRect.w, &ui.scoreRect.h); // Lấy kích thước
    ui.scoreRect.x = 10; ui.scoreRect.y = 10;                  // Đặt vị trí

    // Cập nhật texture điểm cao nhất
    if (ui.highScoreTexture) SDL_DestroyTexture(ui.highScoreTexture);
    std::string highScoreText = "High Score: " + std::to_string(ui.highScore);
    ui.highScoreTexture = createTextTexture(renderer, ui.font, highScoreText, white);
    SDL_QueryTexture(ui.highScoreTexture, nullptr, nullptr, &ui.highScoreRect.w, &ui.highScoreRect.h);
    ui.highScoreRect.x = 10; ui.highScoreRect.y = 40;

    // Cập nhật texture tiêu đề
    if (ui.titleTexture) SDL_DestroyTexture(ui.titleTexture);
    ui.titleTexture = createTextTexture(renderer, ui.titleFont, "Doodle Slash", yellow);
    SDL_QueryTexture(ui.titleTexture, nullptr, nullptr, &ui.titleRect.w, &ui.titleRect.h);
    ui.titleRect.x = (WINDOW_WIDTH - ui.titleRect.w) / 2;      // Canh giữa
    ui.titleRect.y = WINDOW_HEIGHT / 4;

    // Cập nhật texture "Chơi"
    if (ui.playTexture) SDL_DestroyTexture(ui.playTexture);
    ui.playTexture = createTextTexture(renderer, ui.font, "Press SPACE to Play", white);
    SDL_QueryTexture(ui.playTexture, nullptr, nullptr, &ui.playRect.w, &ui.playRect.h);
    ui.playRect.x = (WINDOW_WIDTH - ui.playRect.w) / 2;
    ui.playRect.y = WINDOW_HEIGHT / 2;

    // Cập nhật texture "Thoát"
    if (ui.exitTexture) SDL_DestroyTexture(ui.exitTexture);
    ui.exitTexture = createTextTexture(renderer, ui.font, "Press ESC to Exit", white);
    SDL_QueryTexture(ui.exitTexture, nullptr, nullptr, &ui.exitRect.w, &ui.exitRect.h);
    ui.exitRect.x = (WINDOW_WIDTH - ui.exitRect.w) / 2;
    ui.exitRect.y = WINDOW_HEIGHT / 2 + 40;

    // Cập nhật texture hướng dẫn
    if (ui.instructionsTexture) SDL_DestroyTexture(ui.instructionsTexture);
    ui.instructionsTexture = createTextTexture(renderer, ui.font,
        "Arrows:Move  Space:Jump  Q/E/W:Slash", white);
    SDL_QueryTexture(ui.instructionsTexture, nullptr, nullptr,
        &ui.instructionsRect.w, &ui.instructionsRect.h);
    ui.instructionsRect.x = (WINDOW_WIDTH - ui.instructionsRect.w) / 2;
    ui.instructionsRect.y = WINDOW_HEIGHT - 100;

    // Cập nhật texture "Bạn đã chết"
    if (ui.gameOverTexture) SDL_DestroyTexture(ui.gameOverTexture);
    ui.gameOverTexture = createTextTexture(renderer, ui.font, "You Died!", yellow);
    SDL_QueryTexture(ui.gameOverTexture, nullptr, nullptr, &ui.gameOverRect.w, &ui.gameOverRect.h);
    ui.gameOverRect.x = (WINDOW_WIDTH - ui.gameOverRect.w) / 2;
    ui.gameOverRect.y = WINDOW_HEIGHT / 4;

    // Cập nhật texture "Chơi lại"
    if (ui.playAgainTexture) SDL_DestroyTexture(ui.playAgainTexture);
    ui.playAgainTexture = createTextTexture(renderer, ui.font,
        "R to Replay || ESC to Exit", white);
    SDL_QueryTexture(ui.playAgainTexture, nullptr, nullptr,
        &ui.playAgainRect.w, &ui.playAgainRect.h);
    ui.playAgainRect.x = (WINDOW_WIDTH - ui.playAgainRect.w) / 2;
    ui.playAgainRect.y = WINDOW_HEIGHT / 2;

    // Cập nhật timer boost chém
    if (player.slashBoostTimerTexture) SDL_DestroyTexture(player.slashBoostTimerTexture);
    if (player.slashBoostActive) {
        Uint32 remainingTime = static_cast<Uint32>((SLASH_BOOST_DURATION -
            (SDL_GetTicks() - player.slashBoostStartTime)) / 1000); // Tính thời gian còn lại
        std::string timerText = "Slash Boost: " + std::to_string(remainingTime + 1) + "s";
        player.slashBoostTimerTexture = createTextTexture(renderer, ui.font, timerText, yellow);
        SDL_QueryTexture(player.slashBoostTimerTexture, nullptr, nullptr,
            &player.slashBoostTimerRect.w, &player.slashBoostTimerRect.h);
        player.slashBoostTimerRect.x = 10;
        player.slashBoostTimerRect.y = 70;
    }
    else {
        player.slashBoostTimerTexture = nullptr; // Xóa texture nếu không có boost
    }
}

// Hàm khởi tạo SDL
bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
    // Khởi tạo video và audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ||
        IMG_Init(IMG_INIT_PNG) == 0 ||                        // Khởi tạo tải ảnh PNG
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0 || // Khởi tạo âm thanh
        TTF_Init() == -1) return false;                       // Khởi tạo font

    // Tạo cửa sổ game
    window = SDL_CreateWindow("Doodle Slash", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) return false;

    // Tạo renderer với tăng tốc phần cứng
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != nullptr; // Trả về true nếu thành công
}

// Hàm tạo bệ đỡ
void generatePlatforms(std::vector<Platform>& platforms) {
    platforms.clear();              // Xóa các bệ cũ
    platforms.reserve(NUM_PLATFORMS); // Đặt trước bộ nhớ cho số lượng bệ
    for (int i = 0; i < NUM_PLATFORMS; ++i) {
        // Tạo bệ với vị trí ngẫu nhiên
        Platform p{ static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH)),
                   static_cast<float>(WINDOW_HEIGHT - (i * (WINDOW_HEIGHT / NUM_PLATFORMS))),
                   PLATFORM_WIDTH, PLATFORM_HEIGHT,
                   static_cast<PlatformType>(rand() % 3), // Chọn loại ngẫu nhiên
                   true,                          // Hoạt động
                   (rand() % 2 == 0) ? 1.0f : -1.0f }; // Hướng di chuyển ngẫu nhiên

        // Gán sprite theo loại bệ
        p.sprite = (p.type == NORMAL) ? SPRITE_PLATFORM_NORMAL :
            (p.type == BREAKING) ? SPRITE_PLATFORM_BREAKING :
            SPRITE_PLATFORM_MOVING;

        // Tạo vật phẩm ngẫu nhiên
        p.powerUp.active = (static_cast<float>(rand()) / RAND_MAX) < POWERUP_SPAWN_CHANCE;
        if (p.powerUp.active) {
            PowerUpType type = (rand() % 2 == 0) ? SHIELD : SLASH_BOOST; // Chọn loại ngẫu nhiên
            p.powerUp = { p.x + (p.w - POWERUP_SIZE) / 2.0f, // Canh giữa bệ
                         p.y - POWERUP_SIZE, POWERUP_SIZE, POWERUP_SIZE,
                         true, type,
                         (type == SHIELD) ? SPRITE_SHIELD : SPRITE_SLASH_BOOST };
        }
        platforms.push_back(p); // Thêm bệ vào danh sách
    }
}

// Hàm thử tạo quái vật
void trySpawnEnemy(std::vector<Enemy>& enemies, float spawnChance) {
    // Đếm số quái đang hoạt động
    size_t activeEnemies = std::count_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.active; });
    if (activeEnemies >= MAX_ENEMIES) return; // Không tạo nếu đã đủ

    // Tạo quái với xác suất
    if (static_cast<float>(rand()) / RAND_MAX < spawnChance) {
        Enemy e{ static_cast<float>(rand() % (WINDOW_WIDTH - ENEMY_SIZE)),
                -ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE, true }; // Tạo ở ngoài màn hình
        e.centerX = e.x; e.centerY = e.y; // Đặt trung tâm ban đầu

        int type = rand() % 3; // Chọn loại quái ngẫu nhiên
        if (type == 0) {       // Quái bay
            e.speed = 1.0f + static_cast<float>(rand() % 20) / 10.0f; // Tốc độ ngẫu nhiên
            e.horizontalVelocity = (rand() % 2 == 0) ?
                ENEMY_HORIZONTAL_SPEED : -ENEMY_HORIZONTAL_SPEED; // Hướng ngẫu nhiên
            e.spriteFly[0] = SPRITE_ENEMY_FLY1; // Gán animation
            e.spriteFly[1] = SPRITE_ENEMY_FLY2;
            e.spriteFly[2] = SPRITE_ENEMY_FLY3;
            e.spriteFly[3] = SPRITE_ENEMY_FLY4;
        }
        else if (type == 1) {  // Quái bắn
            e.isShootingMonster = true;
            e.spriteShooting[0] = SPRITE_SHOOTING_MONSTER1;
            e.spriteShooting[1] = SPRITE_SHOOTING_MONSTER2;
            e.spriteShooting[2] = SPRITE_SHOOTING_MONSTER3;
            e.spriteShooting[3] = SPRITE_SHOOTING_MONSTER4;
            e.lastShotTime = SDL_GetTicks();
        }
        else {                // Quái đuổi
            e.isChasingEnemy = true;
            e.spriteChasing[0] = SPRITE_CHASING_ENEMY1;
            e.spriteChasing[1] = SPRITE_CHASING_ENEMY2;
            e.spriteChasing[2] = SPRITE_CHASING_ENEMY3;
            e.spriteChasing[3] = SPRITE_CHASING_ENEMY4;
        }
        enemies.push_back(e); // Thêm quái vào danh sách
    }
}

// Hàm kiểm tra va chạm người chơi và bệ đỡ
inline bool checkCollision(const Player& player, const Platform& platform) {
    return platform.active &&
        player.x + player.w > platform.x &&
        player.x < platform.x + platform.w &&
        player.y + player.h > platform.y &&
        player.y + player.h < platform.y + platform.h + 5 && // Khoảng cách đệm
        player.vy > 0; // Chỉ va chạm khi đang rơi
}

// Hàm kiểm tra va chạm người chơi và quái
inline bool checkPlayerEnemyCollision(const Player& player, const Enemy& enemy) {
    return enemy.active &&
        player.x + player.w > enemy.x &&
        player.x < enemy.x + enemy.w &&
        player.y + player.h > enemy.y &&
        player.y < enemy.y + enemy.h;
}

// Hàm kiểm tra va chạm đòn chém và quái
inline bool checkSlashEnemyCollision(const Slash& slash, const Enemy& enemy) {
    return slash.active && enemy.active &&
        slash.x + slash.w > enemy.x &&
        slash.x < enemy.x + enemy.w &&
        slash.y + slash.h > enemy.y &&
        slash.y < enemy.y + enemy.h;
}

// Hàm kiểm tra va chạm đạn quái và người chơi
inline bool checkMonsterBulletPlayerCollision(const MonsterBullet& bullet, const Player& player) {
    return bullet.active && !bullet.deflected &&
        bullet.x + MONSTER_BULLET_SIZE > player.x &&
        bullet.x < player.x + player.w &&
        bullet.y + MONSTER_BULLET_SIZE > player.y &&
        bullet.y < player.y + player.h;
}

// Hàm kiểm tra va chạm đòn chém và đạn
inline bool checkSlashBulletCollision(const Slash& slash, const MonsterBullet& bullet) {
    return slash.active && bullet.active && !bullet.deflected &&
        slash.x + slash.w > bullet.x &&
        slash.x < bullet.x + MONSTER_BULLET_SIZE &&
        slash.y + slash.h > bullet.y &&
        slash.y < bullet.y + MONSTER_BULLET_SIZE;
}

// Hàm kiểm tra va chạm đạn phản và quái
inline bool checkBulletEnemyCollision(const MonsterBullet& bullet, const Enemy& enemy) {
    return bullet.active && bullet.deflected && enemy.active &&
        bullet.x + MONSTER_BULLET_SIZE > enemy.x &&
        bullet.x < enemy.x + enemy.w &&
        bullet.y + MONSTER_BULLET_SIZE > enemy.y &&
        bullet.y < enemy.y + enemy.h;
}

// Hàm kiểm tra va chạm người chơi và vật phẩm
inline bool checkPlayerPowerUpCollision(const Player& player, const PowerUp& powerUp) {
    return powerUp.active &&
        player.x + player.w > powerUp.x &&
        player.x < powerUp.x + powerUp.w &&
        player.y + player.h > powerUp.y &&
        player.y < powerUp.y + powerUp.h;
}

// Hàm đặt người chơi lên bệ giữa
void spawnPlayerOnMiddlePlatform(Player& player, const std::vector<Platform>& platforms) {
    float middleY = WINDOW_HEIGHT / 2.0f, closestDistance = -1; // Tìm bệ gần giữa nhất
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
    if (middlePlatform) { // Đặt người chơi lên bệ
        player.x = middlePlatform->x + (middlePlatform->w - player.w) / 2.0f;
        player.y = middlePlatform->y - player.h;
        player.vy = 0;
        player.jumpCount = 0;
    }
}

// Hàm thực hiện đòn chém
void performSlash(Player& player, Slash& slash, int direction, Mix_Chunk* slashSound) {
    Uint32 currentTime = SDL_GetTicks(); // Lấy thời gian hiện tại
    int currentSlashDelay = player.slashBoostActive ? SLASH_BOOST_DELAY : SLASH_DELAY; // Chọn delay
    if (currentTime - player.lastSlashTime < currentSlashDelay) return; // Kiểm tra cooldown

    // Tạo đòn chém mới
    Slash newSlash{ 0, 0, 0, 0, true, direction, currentTime, 0, 0, SpriteRect{}, 255.0f };
    switch (direction) {
    case 1: // Chém phải
        newSlash.x = player.x + player.w;
        newSlash.y = player.y + (player.h - SLASH_HEIGHT) / 2;
        newSlash.w = SLASH_WIDTH;
        newSlash.h = SLASH_HEIGHT;
        newSlash.vx = SLASH_SPEED;
        newSlash.sprite = SPRITE_SLASH_RIGHT;
        break;
    case 2: // Chém trái
        newSlash.x = player.x - SLASH_WIDTH;
        newSlash.y = player.y + (player.h - SLASH_HEIGHT) / 2;
        newSlash.w = SLASH_WIDTH;
        newSlash.h = SLASH_HEIGHT;
        newSlash.vx = -SLASH_SPEED;
        newSlash.sprite = SPRITE_SLASH_LEFT;
        break;
    case 3: // Chém lên
        newSlash.x = player.x + (player.w - SLASH_UP_WIDTH) / 2;
        newSlash.y = player.y - SLASH_UP_HEIGHT;
        newSlash.w = SLASH_UP_WIDTH;
        newSlash.h = SLASH_UP_HEIGHT;
        newSlash.vy = -SLASH_SPEED;
        newSlash.sprite = SPRITE_SLASH_UP;
        break;
    default: return;
    }
    slash = newSlash; // Gán đòn chém mới
    player.lastSlashTime = currentTime;
    player.slashDirection = direction;
    player.slashStartTime = currentTime;
    Mix_PlayChannel(-1, slashSound, 0); // Phát âm thanh chém
}

// Hàm quái bắn đạn
void shootMonsterBullet(Enemy& enemy, std::vector<MonsterBullet>& monsterBullets,
    const Player& player, Mix_Chunk* shootSound) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - enemy.lastShotTime < SHOOTING_MONSTER_SHOOT_DELAY) return; // Kiểm tra cooldown

    // Tính hướng bắn về phía người chơi
    float dx = player.x + player.w / 2.0f - (enemy.x + enemy.w / 2.0f);
    float dy = player.y + player.h / 2.0f - (enemy.y + enemy.h / 2.0f);
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance; dy /= distance; // Chuẩn hóa vector
        // Tạo đạn mới
        monsterBullets.push_back({ enemy.x + enemy.w / 2.0f - MONSTER_BULLET_SIZE / 2.0f,
                                 enemy.y + enemy.h / 2.0f,
                                 dx * MONSTER_BULLET_SPEED, dy * MONSTER_BULLET_SPEED,
                                 true, false, SPRITE_MONSTER_BULLET });
        enemy.lastShotTime = currentTime;
        Mix_PlayChannel(-1, shootSound, 0); // Phát âm thanh bắn
    }
}

// Hàm quái đuổi theo người chơi
void chasePlayer(Enemy& enemy, const Player& player) {
    // Tính hướng đuổi
    float dx = player.x + player.w / 2.0f - (enemy.x + enemy.w / 2.0f);
    float dy = player.y + player.h / 2.0f - (enemy.y + enemy.h / 2.0f);
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance; dy /= distance; // Chuẩn hóa vector
        enemy.x += dx * CHASING_ENEMY_SPEED; // Di chuyển theo x
        enemy.y += dy * CHASING_ENEMY_SPEED; // Di chuyển theo y
    }
}

// Hàm reset game
void resetGame(Player& player, std::vector<Platform>& platforms, std::vector<Enemy>& enemies,
    std::vector<MonsterBullet>& monsterBullets, UI& ui, SDL_Renderer* renderer) {
    // Reset người chơi về trạng thái ban đầu
    player = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 100.0f, 0.0f, 25, 40, 0,
              SPRITE_PLAYER_NEUTRAL, 0, 0, false, false, 0, 0, 0, nullptr, {0, 0, 0, 0} };
    if (player.slashBoostTimerTexture) SDL_DestroyTexture(player.slashBoostTimerTexture);
    player.slashBoostTimerTexture = nullptr;

    enemies.clear();         // Xóa quái
    monsterBullets.clear();  // Xóa đạn
    generatePlatforms(platforms); // Tạo lại bệ đỡ
    spawnPlayerOnMiddlePlatform(player, platforms); // Đặt người chơi lên bệ giữa
    ui.highScore = std::max(ui.highScore, ui.currentScore); // Cập nhật điểm cao nhất
    ui.currentScore = ui.totalEnemiesKilled = 0; // Reset điểm và số quái giết
    ui.totalScrollDistance = 0; // Reset khoảng cách cuộn
    ui.startTime = SDL_GetTicks(); // Reset thời gian bắt đầu
    ui.enemySpawnChance = 0.005f; // Reset xác suất tạo quái
    updateUITextures(renderer, ui, player); // Cập nhật giao diện
}

// Hàm chính
int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;     // Con trỏ cửa sổ
    SDL_Renderer* renderer = nullptr; // Con trỏ renderer
    if (!initSDL(window, renderer)) return 1; // Khởi tạo SDL, thoát nếu lỗi

    // Tải atlas sprite
    SDL_Texture* atlasTexture = loadTexture(ATLAS_PATH.c_str(), renderer);
    // Tải nền
    Background background{ loadTexture(BACKGROUND_PATH.c_str(), renderer), 0, 0, 0.0f };
    if (!atlasTexture || !background.texture ||
        SDL_QueryTexture(background.texture, nullptr, nullptr,
            &background.width, &background.height) < 0) {
        SDL_Log("Failed to load textures"); // Báo lỗi nếu tải thất bại
        return 1;
    }

    // Tải âm thanh
    Mix_Chunk* jumpSound = Mix_LoadWAV("/project/data/audio/hit.wav");      // Âm thanh nhảy
    Mix_Chunk* slashSound = Mix_LoadWAV("/project/data/audio/hit.wav");    // Âm thanh chém
    Mix_Chunk* shootSound = Mix_LoadWAV("/project/data/audio/hit.wav");    // Âm thanh bắn
    Mix_Chunk* powerUpSound = Mix_LoadWAV("/project/data/audio/hit.wav"); // Âm thanh nhặt vật phẩm
    Mix_Chunk* deathSound = Mix_LoadWAV("/project/data/audio/hit.wav");    // Âm thanh chết
    Mix_Music* backgroundMusic = Mix_LoadMUS(MUSIC_PATH.c_str()); // Nhạc nền
    if (!jumpSound || !slashSound || !shootSound || !powerUpSound ||
        !deathSound || !backgroundMusic) {
        SDL_Log("Failed to load audio: %s", Mix_GetError());
        return 1;
    }

    // Khởi tạo UI
    UI ui{ TTF_OpenFont(FONT_PATH.c_str(), 24),      // Font nhỏ
          TTF_OpenFont(FONT_PATH.c_str(), 48),       // Font lớn cho tiêu đề
          nullptr, {0, 0, 0, 0},                     // Score
          nullptr, {0, 0, 0, 0},                     // High score
          nullptr, {0, 0, 0, 0},                     // Title
          nullptr, {0, 0, 0, 0},                     // Play
          nullptr, {0, 0, 0, 0},                     // Exit
          nullptr, {0, 0, 0, 0},                     // Game over
          nullptr, {0, 0, 0, 0},                     // Play again
          nullptr, {0, 0, 0, 0},                     // Instructions
          0, 0, 0, 0, 0, 0.005f };                  // Các giá trị số
    if (!ui.font || !ui.titleFont) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return 1;
    }

    srand(static_cast<unsigned>(time(nullptr))); // Khởi tạo seed random
    // Khởi tạo người chơi
    Player player{ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 100.0f, 0.0f, 25, 40, 0,
                  SPRITE_PLAYER_NEUTRAL, 0, 0, false, false, 0, 0, 0, nullptr, {0, 0, 0, 0} };
    std::vector<Platform> platforms; // Danh sách bệ đỡ
    Slash slash{ 0, 0, 0, 0, false, 0, 0, 0, 0, SPRITE_SLASH_RIGHT, 255.0f }; // Đòn chém ban đầu
    std::vector<Enemy> enemies;      // Danh sách quái
    std::vector<MonsterBullet> monsterBullets; // Danh sách đạn quái
    generatePlatforms(platforms);    // Tạo bệ đỡ ban đầu
    spawnPlayerOnMiddlePlatform(player, platforms); // Đặt người chơi lên bệ giữa
    updateUITextures(renderer, ui, player); // Cập nhật giao diện ban đầu

    Mix_PlayMusic(backgroundMusic, -1); // Phát nhạc nền lặp vô hạn

    bool quit = false;            // Cờ thoát game
    GameState gameState = START_MENU; // Trạng thái game ban đầu
    SDL_Event e;                  // Biến sự kiện SDL
    float animationTime = 0.0f;   // Thời gian cho animation

    // Vòng lặp game chính
    while (!quit) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true; // Thoát khi đóng cửa sổ
            if (e.type == SDL_KEYDOWN) {         // Xử lý phím bấm
                switch (gameState) {
                case START_MENU:
                    if (e.key.keysym.sym == SDLK_SPACE) { // Phím Space để chơi
                        gameState = PLAYING;
                        ui.startTime = SDL_GetTicks();
                    }
                    if (e.key.keysym.sym == SDLK_ESCAPE) quit = true; // Phím Esc để thoát
                    break;
                case PLAYING:
                    switch (e.key.keysym.sym) {
                    case SDLK_SPACE: // Nhảy
                        if (player.jumpCount < 1) {
                            player.vy = JUMP_VELOCITY;
                            player.jumpCount++;
                            Mix_PlayChannel(-1, jumpSound, 0);
                        }
                        break;
                    case SDLK_e: performSlash(player, slash, 1, slashSound); break; // Chém phải
                    case SDLK_q: performSlash(player, slash, 2, slashSound); break; // Chém trái
                    case SDLK_w: performSlash(player, slash, 3, slashSound); break; // Chém lên
                    }
                    break;
                case GAME_OVER_MENU:
                    if (e.key.keysym.sym == SDLK_r) { // Phím R để chơi lại
                        resetGame(player, platforms, enemies, monsterBullets, ui, renderer);
                        gameState = PLAYING;
                    }
                    if (e.key.keysym.sym == SDLK_ESCAPE) quit = true; // Phím Esc để thoát
                    break;
                }
            }
        }

        Uint32 currentTime = SDL_GetTicks(); // Lấy thời gian hiện tại

        if (gameState == PLAYING) {
            animationTime += 0.016f; // Cập nhật thời gian animation (60 FPS)
            ui.enemySpawnChance = std::min(ui.enemySpawnChance + 0.00001f, 0.05f); // Tăng xác suất quái

            // Tính điểm
            ui.currentScore = static_cast<int>(ui.totalScrollDistance * SCORE_PER_SCROLL_UNIT) +
                ui.totalEnemiesKilled * SCORE_PER_ENEMY;
            updateUITextures(renderer, ui, player); // Cập nhật giao diện

            // Kiểm tra hết boost chém
            if (player.slashBoostActive && currentTime - player.slashBoostStartTime > SLASH_BOOST_DURATION) {
                player.slashBoostActive = false;
            }

            // Xử lý di chuyển người chơi
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

            // Cập nhật đòn chém
            if (slash.active) {
                slash.x += slash.vx;
                slash.y += slash.vy; // Di chuyển
                slash.alpha = 255.0f * (1.0f - static_cast<float>(currentTime - slash.startTime) / SLASH_DURATION); // Giảm alpha
                if (currentTime - slash.startTime > SLASH_DURATION) { // Hết thời gian
                    slash.active = false;
                    player.slashDirection = 0;
                }
            }

            // Giới hạn vị trí người chơi trong màn hình
            player.x = (player.x + player.w < 0) ? WINDOW_WIDTH - player.w :
                (player.x > WINDOW_WIDTH) ? 0 : player.x;
            player.vy += GRAVITY; // Áp dụng trọng lực
            player.y += player.vy; // Cập nhật vị trí y

            // Cuộn nền
            background.y = static_cast<float>(fmod(background.y + 0.7f,
                static_cast<float>(background.height)));

            // Xử lý bệ đỡ
            for (auto& platform : platforms) {
                if (platform.type == MOVING && platform.active) { // Bệ di động
                    platform.x += MOVING_PLATFORM_SPEED * platform.direction;
                    if (platform.x <= 0 || platform.x + platform.w >= WINDOW_WIDTH)
                        platform.direction *= -1; // Đổi hướng khi chạm biên
                    if (platform.powerUp.active)
                        platform.powerUp.x = platform.x + (platform.w - POWERUP_SIZE) / 2.0f; // Cập nhật vị trí vật phẩm
                }
                if (checkCollision(player, platform)) { // Va chạm với người chơi
                    player.vy = JUMP_VELOCITY;
                    player.jumpCount = 0;
                    if (platform.type == BREAKING) platform.active = false; // Phá bệ vỡ
                    Mix_PlayChannel(-1, jumpSound, 0);
                }
                if (checkPlayerPowerUpCollision(player, platform.powerUp)) { // Nhặt vật phẩm
                    platform.powerUp.active = false;
                    Mix_PlayChannel(-1, powerUpSound, 0);
                    if (platform.powerUp.type == SHIELD) player.shieldActive = true;
                    else if (platform.powerUp.type == SLASH_BOOST) {
                        player.slashBoostActive = true;
                        player.slashBoostStartTime = currentTime;
                    }
                }
            }

            // Cập nhật quái vật
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                if (enemy.isShootingMonster)
                    shootMonsterBullet(enemy, monsterBullets, player, shootSound); // Quái bắn
                else if (enemy.isChasingEnemy)
                    chasePlayer(enemy, player); // Quái đuổi
                else { // Quái bay
                    enemy.centerX += enemy.horizontalVelocity;
                    enemy.x = std::max(0.0f, std::min(enemy.centerX,
                        static_cast<float>(WINDOW_WIDTH - enemy.w))); // Giới hạn x
                    if (enemy.x <= 0 || enemy.x + enemy.w >= WINDOW_WIDTH)
                        enemy.horizontalVelocity *= -1; // Đổi hướng
                    enemy.y = enemy.centerY + sinf(animationTime * enemy.speed) * 50.0f; // Chuyển động sóng
                }
            }

            // Xử lý va chạm với quái
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                if (checkPlayerEnemyCollision(player, enemy)) {
                    if (player.vy > 0) { // Người chơi rơi xuống quái
                        enemy.active = false;
                        player.vy = JUMP_VELOCITY;
                        player.jumpCount = 0;
                        Mix_PlayChannel(-1, jumpSound, 0);
                        ++ui.totalEnemiesKilled;
                    }
                    else if (player.shieldActive) player.shieldActive = false; // Mất khiên
                    else { // Chết
                        Mix_PlayChannel(-1, deathSound, 0);
                        gameState = GAME_OVER_MENU;
                    }
                }
                if (slash.active && checkSlashEnemyCollision(slash, enemy)) { // Chém trúng quái
                    enemy.active = false;
                    player.jumpCount = 0;
                    ++ui.totalEnemiesKilled;
                }
            }

            // Cập nhật đạn quái
            for (auto& bullet : monsterBullets) {
                if (!bullet.active) continue;
                bullet.x += bullet.vx;
                bullet.y += bullet.vy; // Di chuyển đạn
                if (bullet.y > WINDOW_HEIGHT || bullet.x < 0 || bullet.x > WINDOW_WIDTH)
                    bullet.active = false; // Xóa đạn ra ngoài màn hình
                if (checkMonsterBulletPlayerCollision(bullet, player)) { // Trúng người chơi
                    bullet.active = false;
                    if (player.shieldActive) player.shieldActive = false;
                    else {
                        Mix_PlayChannel(-1, deathSound, 0);
                        gameState = GAME_OVER_MENU;
                    }
                }
                if (slash.active && checkSlashBulletCollision(slash, bullet)) { // Phản đạn
                    bullet.deflected = true;
                    bullet.vx = -bullet.vx;
                    bullet.vy = -bullet.vy;
                }
                for (auto& enemy : enemies) { // Đạn phản trúng quái
                    if (checkBulletEnemyCollision(bullet, enemy)) {
                        bullet.active = false;
                        enemy.active = false;
                        ++ui.totalEnemiesKilled;
                    }
                }
            }

            // Cuộn màn hình khi người chơi lên cao
            if (player.y < WINDOW_HEIGHT / 2.5f) {
                float offset = (WINDOW_HEIGHT / 2.5f) - player.y; // Tính độ lệch
                player.y = WINDOW_HEIGHT / 2.5f; // Giữ người chơi ở giữa
                ui.totalScrollDistance += offset; // Cập nhật khoảng cách cuộn
                background.y += offset; // Cuộn nền
                for (auto& platform : platforms) { // Cuộn bệ đỡ
                    platform.y += offset;
                    if (platform.powerUp.active) platform.powerUp.y += offset;
                    if (platform.y > WINDOW_HEIGHT) { // Tái tạo bệ dưới màn hình
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
                if (slash.active) slash.y += offset; // Cuộn đòn chém
                for (auto& enemy : enemies)
                    if (enemy.active) { enemy.centerY += offset; enemy.y += offset; } // Cuộn quái
                for (auto& bullet : monsterBullets)
                    if (bullet.active) bullet.y += offset; // Cuộn đạn
                trySpawnEnemy(enemies, ui.enemySpawnChance); // Thử tạo quái mới
            }

            // Xóa quái ra khỏi màn hình
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                [](const Enemy& e) { return e.active && e.y > WINDOW_HEIGHT; }), enemies.end());

            // Kiểm tra người chơi rơi khỏi màn hình
            if (player.y + player.h > WINDOW_HEIGHT) {
                Mix_PlayChannel(-1, deathSound, 0);
                gameState = GAME_OVER_MENU;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Đặt màu nền trắng
        SDL_RenderClear(renderer); // Xóa màn hình
        // Vẽ nền (2 phần để lặp liền mạch)
        SDL_Rect bgDst1 = { 0, static_cast<int>(background.y - background.height),
                          WINDOW_WIDTH, background.height };
        SDL_Rect bgDst2 = { 0, static_cast<int>(background.y),
                          WINDOW_WIDTH, background.height };
        SDL_RenderCopy(renderer, background.texture, nullptr, &bgDst1);
        SDL_RenderCopy(renderer, background.texture, nullptr, &bgDst2);

        // Biến render tạm thời
        SDL_Rect playerSrc, playerDst;
        SDL_Rect shieldSrc, shieldDst;
        SDL_Rect slashSrc, slashDst;

        // Render theo trạng thái game
        switch (gameState) {
        case START_MENU:
            SDL_RenderCopy(renderer, ui.titleTexture, nullptr, &ui.titleRect); // Vẽ tiêu đề
            SDL_RenderCopy(renderer, ui.playTexture, nullptr, &ui.playRect);   // Vẽ "Chơi"
            SDL_RenderCopy(renderer, ui.exitTexture, nullptr, &ui.exitRect);   // Vẽ "Thoát"
            SDL_RenderCopy(renderer, ui.instructionsTexture, nullptr,
                &ui.instructionsRect); // Vẽ hướng dẫn
            break;
        case PLAYING:
            // Vẽ người chơi
            playerSrc = player.sprite.toSDLRect();
            playerDst = { static_cast<int>(player.x), static_cast<int>(player.y),
                         player.w, player.h };
            SDL_RenderCopy(renderer, atlasTexture, &playerSrc, &playerDst);

            // Vẽ khiên nếu có
            if (player.shieldActive) {
                shieldSrc = SPRITE_SHIELD_EQUIPPED.toSDLRect();
                shieldDst = { static_cast<int>(player.x - (SPRITE_SHIELD_EQUIPPED.w - player.w) / 2),
                             static_cast<int>(player.y - (SPRITE_SHIELD_EQUIPPED.h - player.h) / 2),
                             SPRITE_SHIELD_EQUIPPED.w, SPRITE_SHIELD_EQUIPPED.h };
                SDL_RenderCopy(renderer, atlasTexture, &shieldSrc, &shieldDst);
            }

            // Vẽ đòn chém
            if (slash.active) {
                slashSrc = slash.sprite.toSDLRect();
                slashDst = { static_cast<int>(slash.x), static_cast<int>(slash.y),
                           slash.w, slash.h };
                SDL_SetTextureAlphaMod(atlasTexture, static_cast<Uint8>(slash.alpha)); // Đặt độ trong suốt
                SDL_RenderCopy(renderer, atlasTexture, &slashSrc, &slashDst);
                SDL_SetTextureAlphaMod(atlasTexture, 255); // Reset alpha
            }

            // Vẽ bệ đỡ và vật phẩm
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

            // Vẽ đạn quái
            for (const auto& bullet : monsterBullets) {
                if (bullet.active) {
                    SDL_Rect src = bullet.sprite.toSDLRect();
                    SDL_Rect dst = { static_cast<int>(bullet.x), static_cast<int>(bullet.y),
                                   MONSTER_BULLET_SIZE, MONSTER_BULLET_SIZE };
                    SDL_RenderCopy(renderer, atlasTexture, &src, &dst);
                }
            }

            // Vẽ quái với animation
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

            // Vẽ giao diện
            SDL_RenderCopy(renderer, ui.scoreTexture, nullptr, &ui.scoreRect);
            SDL_RenderCopy(renderer, ui.highScoreTexture, nullptr, &ui.highScoreRect);
            if (player.slashBoostActive && player.slashBoostTimerTexture) {
                SDL_RenderCopy(renderer, player.slashBoostTimerTexture, nullptr,
                    &player.slashBoostTimerRect); // Vẽ timer boost
            }
            break;
        case GAME_OVER_MENU:
            SDL_RenderCopy(renderer, ui.gameOverTexture, nullptr, &ui.gameOverRect); // Vẽ "Bạn đã chết"
            SDL_RenderCopy(renderer, ui.scoreTexture, nullptr, &ui.scoreRect);       // Vẽ điểm
            SDL_RenderCopy(renderer, ui.highScoreTexture, nullptr, &ui.highScoreRect); // Vẽ điểm cao nhất
            SDL_RenderCopy(renderer, ui.playAgainTexture, nullptr, &ui.playAgainRect); // Vẽ "Chơi lại"
            break;
        }

        SDL_RenderPresent(renderer); // Hiển thị frame
        SDL_Delay(16); // Giới hạn 60 FPS
    }

    // Giải phóng tài nguyên
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
    Mix_Quit();          // Thoát SDL_mixer
    TTF_Quit();          // Thoát SDL_ttf
    IMG_Quit();          // Thoát SDL_image
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();          // Thoát SDL

    return 0; // Kết thúc chương trình
}