# Buzzer Driver

Driver điều khiển còi buzzer trên **AK Embedded Base Kit (STM32L1)**.  
Sử dụng **PWM timer** để tạo âm thanh và hỗ trợ phát chuỗi nốt nhạc (melody sequence).

---

## Các file

| File | Mô tả |
|------|-------|
| `buzzer.h` | Header — khai báo API công khai |
| `buzzer.c` | Source — triển khai driver và interrupt handler |
| `buzzer_music.h` | Header — định nghĩa kiểu dữ liệu nốt nhạc và danh sách âm thanh |
| `buzzer_music.c` | Source — dữ liệu chuỗi nốt nhạc (melody data) |

---

## Cơ chế hoạt động

```
BUZZER_PlaySound()
       │
       ▼
buzzer_get_music()   ← Tra bảng buzzer_music_table để lấy con trỏ tones[]
       │
       ▼
BUZZER_PlayTones()   ← Thiết lập con trỏ chuỗi nốt, bật _tones_playing
       │
       ▼
BUZZER_Enable()      ← Cấu hình PWM timer theo tần số nốt đầu tiên
       │
       ▼
  [Timer IRQ]        ← Mỗi tick ngắt: giảm _beep_duration
       │              Khi = 0 → chuyển sang nốt tiếp theo
       ▼
BUZZER_Disable()     ← Khi hết chuỗi nốt, tắt PWM và pin về analog
```

Toàn bộ việc chuyển nốt nhạc được thực hiện **hoàn toàn trong ngắt** (`buzzer_irq`), không block task nào.

---

## API

### `BUZZER_Init(void)`
Khởi tạo driver: cấu hình GPIO, PWM timer và NVIC interrupt.  
Gọi **một lần duy nhất** trong `app_bsp.cpp` khi khởi động.

---

### `BUZZER_Enable(uint16_t freq, uint32_t duration)`
Bật buzzer phát âm thanh thô ở tần số và độ dài chỉ định.

| Tham số | Kiểu | Mô tả |
|---------|------|-------|
| `freq` | `uint16_t` | Tần số âm thanh (Hz). Hợp lệ: **100 – 8000 Hz** |
| `duration` | `uint32_t` | Độ dài (đơn vị **10 ms**). VD: `5` = 50 ms |

> ⚠️ Nếu `freq < 100` hoặc `freq > 8000` hoặc `duration == 0`, hàm sẽ tự gọi `BUZZER_Disable()`.

---

### `BUZZER_Disable(void)`
Tắt buzzer: dừng timer, tắt clock peripheral, đặt pin về analog để tiết kiệm điện.

---

### `BUZZER_PlaySound(buzzer_sound_t sound)`
Phát một âm thanh **một lần** (không lặp lại).

```c
BUZZER_PlaySound(BUZZER_SOUND_CLICK);
BUZZER_PlaySound(BUZZER_SOUND_GAMEOVER);
```

---

### `BUZZER_PlaySoundLoop(buzzer_sound_t sound)`
Phát âm thanh **lặp lại liên tục** (dùng cho nhạc nền).

```c
BUZZER_PlaySoundLoop(BUZZER_SOUND_PIRATES);
BUZZER_PlaySoundLoop(BUZZER_SOUND_SUPER_MARIO);
```

> 💡 Khi đang loop nhạc nền, nếu gọi `BUZZER_PlaySound()` để phát hiệu ứng âm thanh, driver sẽ tự động **tạm dừng nhạc nền** và **tiếp tục lại** sau khi âm thanh kết thúc (nhờ cơ chế `_bg_tones_saved`).

---

### `BUZZER_StopLoop(void)`
Dừng lặp nhạc nền. Âm thanh hiện tại sẽ kết thúc bình thường rồi dừng hẳn.

```c
BUZZER_StopLoop();
```

---

### `BUZZER_Silent(bool isSilent)`
Bật/tắt chế độ im lặng toàn bộ driver.

| Tham số | Giá trị | Ý nghĩa |
|---------|---------|---------|
| `isSilent` | `BUZZER_SILENT_ON` (`0`) | Tắt âm thanh |
| `isSilent` | `BUZZER_SILENT_OFF` (`1`) | Bật âm thanh (mặc định) |

```c
BUZZER_Silent(BUZZER_SILENT_ON);   // Tắt tiếng
BUZZER_Silent(BUZZER_SILENT_OFF);  // Bật tiếng
```

---

## Danh sách âm thanh (`buzzer_sound_t`)

| Enum | Mô tả |
|------|-------|
| `BUZZER_SOUND_CLICK` | Tiếng click ngắn (nhấn nút) |
| `BUZZER_SOUND_DOOM` | Nhạc Doom |
| `BUZZER_SOUND_PEW` | Tiếng bắn laser |
| `BUZZER_SOUND_GAMEOVER` | Nhạc game over |
| `BUZZER_SOUND_PIRATES` | Pirates of the Caribbean |
| `BUZZER_SOUND_BANG` | Tiếng nổ |
| `BUZZER_SOUND_USB_CONNECTED` | Thông báo cắm USB |
| `BUZZER_SOUND_USB_DISCONNECTED` | Thông báo rút USB |
| `BUZZER_SOUND_LETS_GO` | Tiếng bắt đầu game |
| `BUZZER_SOUND_STARTUP` | Nhạc khởi động board |
| `BUZZER_SOUND_3BEEP` | 3 tiếng beep |
| `BUZZER_SOUND_WELCOME` | Nhạc chào mừng |
| `BUZZER_SOUND_GOODBYE` | Nhạc tạm biệt |
| `BUZZER_SOUND_HIGHSCORE` | Đạt điểm cao |
| `BUZZER_SOUND_LOWSCORE` | Điểm thấp |
| `BUZZER_SOUND_SUPER_MARIO` | Super Mario Bros theme |
| `BUZZER_SOUND_MERRY_CHRISTMAS` | Jingle Bells / Merry Christmas |
| `BUZZER_SOUND_TONE_1..7` | Âm đơn tùy chỉnh (tone 1 → 7) |

---

## Cấu trúc dữ liệu nốt nhạc

```c
// Một nốt nhạc
typedef struct {
    uint16_t frequency;  // Tần số (Hz). 0 = khoảng lặng (silence)
    uint8_t  duration;   // Độ dài (đơn vị 10 ms)
} Tone_TypeDef;

// Kết thúc chuỗi nốt: phần tử cuối có frequency = 0 và duration = 0
```

**Ví dụ tự định nghĩa melody:**

```c
const Tone_TypeDef my_melody[] = {
    {440, 10},  // La4, 100ms
    {494, 10},  // Si4, 100ms
    {523, 20},  // Do5, 200ms
    {  0,  5},  // Im lặng 50ms
    {659, 30},  // Mi5, 300ms
    {  0,  0},  // Kết thúc
};
```

---

## Ví dụ sử dụng

```c
#include "buzzer.h"

// Khởi tạo (trong app_bsp.cpp)
BUZZER_Init();

// Phát hiệu ứng âm thanh khi bắn đạn
BUZZER_PlaySound(BUZZER_SOUND_PEW);

// Bật nhạc nền Pirates khi vào game
BUZZER_PlaySoundLoop(BUZZER_SOUND_PIRATES);

// Phát game over (tự pause nhạc nền, resume sau khi xong)
BUZZER_PlaySound(BUZZER_SOUND_GAMEOVER);

// Dừng nhạc nền khi thoát game
BUZZER_StopLoop();
```

---

## Lưu ý quan trọng

- `buzzer_irq()` phải được gọi từ **Timer ISR** trong `io_cfg.c`. Không gọi trực tiếp từ task.
- Tất cả biến nội bộ (`_tones`, `_beep_duration`...) đều là `volatile` — an toàn khi truy cập từ ISR.
- Driver **không sử dụng bộ nhớ động** (no malloc), phù hợp cho embedded.
- Tần số hợp lệ: **100 Hz – 8000 Hz**. Ngoài dải này sẽ bị tự động tắt.
