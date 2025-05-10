# Họ và tên:Bùi Anh Tuấn
# MSV:24022839
## Slash Hopper 2
Slash Hopper 2 là một game platform 2D lấy cảm hứng từ **Doodle Jump**, nhưng được cải tiến với hệ thống chiến đấu hấp dẫn và đồ họa pixel. Người chơi không chỉ nhảy để tránh chướng ngại vật mà còn có thể **chém kẻ thù theo ba hướng** và sử dụng các **vật phẩm tăng sức mạnh** để sống sót và ghi điểm cao nhất.

## 🕹️ Gameplay & Tính năng chính

### 🗡️ Hệ thống chém (Slash Mechanic)
- Người chơi có thể tấn công quái vật theo **3 hướng**: trái (Q), phải (E), lên (W).
- Phản lại đạn quái bắn, giúp tạo chiều sâu chiến đấu.
- Được thực hiện thông qua hàm `performSlash()`.

### ⚡ Vật phẩm tăng sức mạnh (Power-Ups)
- **Shield**: Bảo vệ người chơi khỏi một đòn.
- **Slash Boost**: Tăng tốc độ tấn công.
- Sinh ngẫu nhiên qua hàm `generatePlatforms()` với biến `POWERUP_SPAWN_CHANCE`.

---

## 🧠 Lối chơi lấy cảm hứng từ Doodle Jump

**Slash Hopper 2** lấy nền tảng nhảy từ **Doodle Jump** nhưng kết hợp thêm yếu tố hành động, biến nó thành một trải nghiệm vừa thử thách vừa thú vị.

---

## 📁 Cấu trúc dự án

- `Player`: Vị trí, vận tốc, trạng thái buff và sprite.
- `Slash`: Vị trí, hướng, thời gian tồn tại, độ mờ.
- `Enemy`: 3 loại quái (bay, bắn, đuổi) với sprite riêng.
- `Platform`: 3 loại bệ (thường, vỡ, di động), có thể chứa Power-Up.
- `UI`: Quản lý điểm, điểm cao nhất, font, và hiển thị.
- `Background`: Nền cuộn không giới hạn.

---

## 🔄 Luồng hoạt động chính

1. **Khởi tạo**: `initSDL()` mở cửa sổ, renderer, âm thanh, font.
2. **Main Loop**:
   - Xử lý sự kiện: SDL_PollEvent.
   - Cập nhật logic: chuyển động, va chạm, AI quái.
   - Cuộn màn hình: Khi người chơi lên cao.
   - Render: Vẽ tất cả đối tượng và UI.
3. **Cleanup**: Giải phóng tài nguyên.

---

## 🧰 Kỹ thuật & Công nghệ

- **Ngôn ngữ**: C++  
- **Thư viện**:  
  - SDL2 (graphics, input)
  - SDL_image (ảnh PNG)
  - SDL_mixer (âm thanh WAV, MP3)
  - SDL_ttf (font .ttf)

- **Kỹ thuật đặc trưng**:
  - Vòng lặp game 3 pha: Input → Logic → Render
  - Trạng thái game: `START_MENU`, `PLAYING`, `GAME_OVER_MENU`
  - Cuộn màn hình: dịch toàn bộ vật thể xuống khi nhân vật đi lên
  - AABB Collision Detection
  - AI cơ bản: quái bay hình sin, quái đuổi, quái bắn
  - Animation frame theo thời gian
  - Tạo số ngẫu nhiên: `rand()` + `srand(time(nullptr))`
  - STL `vector` cho enemy, platform, bullets

---

### Yêu cầu:
- SDL2
- SDL_image
- SDL_mixer
- SDL_ttf

