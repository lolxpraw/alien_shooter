# 📁 Giải thích cấu trúc thư mục Project `test_game`

Đây là project game chạy trên vi điều khiển **STM32L1** (kit AK Embedded Base Kit), sử dụng **AK Framework** — một RTOS nhỏ với mô hình lập trình dựa trên **task + message**.

---

## 🗂️ Cấu trúc tổng quan

```
test_game/
├── hardware/        ← Tài liệu phần cứng (sơ đồ mạch, ảnh, BOM)
├── boot/            ← Chương trình bootloader (nạp firmware OTA)
├── application/     ← Chương trình chính (game + toàn bộ logic)
└── .vscode/         ← Cấu hình editor (không ảnh hưởng code)
```

---

## 🔧 `hardware/` — Tài liệu phần cứng

> Không chứa code, chỉ chứa tài liệu thiết kế phần cứng.

| Folder | Nội dung |
|--------|----------|
| `schematic/` | File sơ đồ mạch điện (`.pdf`) — phiên bản 2 và 3 của AK Base Kit |
| `images/` | Ảnh chụp board mạch thực tế |
| `board-assembly/` | Tài liệu lắp ráp linh kiện lên board |
| `manufacturing/` | File dùng cho nhà máy sản xuất PCB (Gerber, BOM...) |
| `bin/` | File binary firmware mẫu đã build sẵn |

---

## 🥾 `boot/` — Bootloader

> Chương trình **khởi động đầu tiên** khi cấp nguồn cho board. Nhiệm vụ chính là kiểm tra xem có firmware mới cần nạp không (OTA update), nếu không thì nhảy sang chạy `application`.

### `boot/sources/` chứa:

| Folder/File | Chức năng |
|-------------|-----------|
| `app/` | Logic nghiệp vụ của bootloader (kiểm tra flash, quyết định jump sang app) |
| `driver/` | Driver phần cứng tối thiểu cần cho boot (flash, UART...) |
| `platform/` | Cấu hình phần cứng STM32 cho bootloader |
| `sys/` | Hệ thống thấp nhất: khởi tạo chip, debug |
| `common/` | Các tiện ích dùng chung |
| `Makefile` | Script biên dịch bootloader |
| `stm32l_init.gdb` | Script GDB để nạp/debug qua ST-Link |

---

## 📱 `application/` — Ứng dụng chính (Game)

> Đây là phần **quan trọng nhất** — chứa toàn bộ code game và logic hệ thống.

```
application/
├── Makefile
├── stm32l_init.gdb
├── build_ak-base-kit-stm32l151-application/   ← Thư mục output sau khi build
└── sources/                                    ← Toàn bộ source code
    ├── ak/         ← AK Framework (RTOS nhỏ)
    ├── app/        ← Logic ứng dụng / game chính
    ├── common/     ← Tiện ích dùng chung
    ├── driver/     ← Driver cho ngoại vi (màn hình, buzzer, nút...)
    ├── libraries/  ← Thư viện bên thứ 3
    ├── networks/   ← Giao thức mạng (Zigbee, RF, Modbus)
    ├── platform/   ← Cấu hình phần cứng STM32
    └── sys/        ← Hệ thống cấp thấp
```

---

### ⚙️ `sources/ak/` — AK Framework (RTOS)

> Đây là **"trái tim"** của hệ thống. AK là một mini-RTOS tự phát triển, cho phép chạy nhiều **task** song song theo mô hình **event-driven** (nhận và xử lý message).

| Folder | Nội dung |
|--------|----------|
| `inc/` | Header files: định nghĩa Task, Message, Timer, Signal... |
| `src/` | Source code của AK kernel |
| `doc/` | Tài liệu về AK framework |
| `ak.cfg.mk` | File cấu hình chọn tính năng của AK khi build |

---

### 🎮 `sources/app/` — Logic ứng dụng (Game chính)

> Đây là nơi chứa **toàn bộ code game và task điều hành**.

#### Files quan trọng:

| File | Chức năng |
|------|-----------|
| `app.cpp / app.h` | Điểm khởi đầu ứng dụng, khởi tạo các task |
| `task_list.cpp / .h` | **Danh sách tất cả task** trong hệ thống (khai báo task ID, priority) |
| `task_display.cpp` | Task quản lý việc vẽ màn hình OLED |
| `task_system.cpp` | Task hệ thống (reset, watchdog...) |
| `task_life.cpp` | Task "nhịp tim" — LED nhấp nháy để biết board còn sống |
| `task_shell.cpp` | Task xử lý lệnh debug qua UART (gõ lệnh như terminal) |
| `task_fw.cpp` | Task cập nhật firmware (OTA) |
| `task_zigbee.cpp` | Task giao tiếp Zigbee không dây |
| `app_data.cpp / .h` | Dữ liệu toàn cục của ứng dụng |
| `app_bsp.cpp / .h` | Board Support Package — khởi tạo phần cứng cho app |
| `shell.cpp` | Triển khai các lệnh debug qua serial |

#### Folder `screens/` — Các màn hình hiển thị:

> Mỗi file `scr_*.cpp` là **một màn hình** trên OLED display.

| File | Màn hình |
|------|----------|
| `scr_startup.cpp` | Màn hình khởi động (logo khi mới bật) |
| `scr_welcome.cpp` | Màn hình chào mừng |
| `scr_idle.cpp` | Màn hình chờ (idle screen) |
| `scr_game.cpp` | **Màn hình game chính** (Space Invaders!) |
| `scr_image.cpp` | Màn hình hiển thị ảnh bitmap |
| `scr_qrcode.cpp` | Màn hình hiển thị QR code |
| `screens_bitmap.cpp` | Lưu trữ các ảnh bitmap dùng trong game |
| `screens.h` | Liệt kê tất cả màn hình, dùng `screen_manager` điều hướng |

---

### 🛠️ `sources/driver/` — Driver phần cứng

> Mỗi **sub-folder** là driver cho một linh kiện cụ thể trên board.

| Folder | Thiết bị | Chức năng |
|--------|----------|-----------|
| `Adafruit_oled_drv/` | Màn hình OLED SSD1306 | Vẽ pixel, chữ, hình lên màn hình |
| `buzzer/` | Còi buzzer | Phát nhạc, âm thanh game |
| `button/` | Các nút bấm | Đọc trạng thái nút, chống rung (debounce) |
| `led/` | LED | Điều khiển bật/tắt LED |
| `gpio/` | GPIO chung | Cấu hình và đọc/ghi các chân GPIO |
| `eeprom/` | EEPROM nội | Lưu dữ liệu không mất khi tắt nguồn |
| `flash/` | Flash nội | Lưu firmware OTA vào flash |
| `nRF24/` | Module RF nRF24L01 | Truyền nhận dữ liệu không dây |
| `AsyncDelay/` | Timer không chặn | Tạo delay mà không block task khác |

#### Chi tiết `buzzer/`:

| File | Chức năng |
|------|-----------|
| `buzzer.c / .h` | Điều khiển phần cứng PWM tạo âm thanh |
| `buzzer_music.c / .h` | Lưu danh sách bài nhạc (Pirates of Caribbean, Mario...) và phát theo sequence note |

---

### 🔌 `sources/platform/stm32l/` — Cấu hình STM32

> Lớp trung gian giữa driver và phần cứng STM32 thực tế.

| File | Chức năng |
|------|-----------|
| `io_cfg.c / .h` | **Cấu hình tất cả chân IO** (SPI, I2C, UART, PWM...) |
| `sys_cfg.c / .h` | Cấu hình clock, timer, interrupt hệ thống |
| `system.c / .h` | Hàm khởi tạo chip STM32 |
| `system_stm32l1xx.c` | File init chuẩn của ST cho STM32L1 |
| `platform.c / .h` | Abstraction layer cho platform |
| `ak.ld` | Linker script — phân vùng Flash/RAM cho chương trình |
| `mini_cpp.cpp` | Hỗ trợ tối thiểu cho C++ trên bare-metal (new/delete) |
| `Libraries/` | HAL/SPL library của STMicroelectronics |
| `arduino/` | Lớp tương thích Arduino (cho một số driver viết theo chuẩn Arduino) |

---

### 🌐 `sources/networks/` — Giao thức truyền thông

| Folder | Giao thức | Chức năng |
|--------|-----------|-----------|
| `ArduinoZigBee/` | Zigbee | Mạng không dây mesh |
| `rf_protocols/` | RF tùy chỉnh | Giao thức RF qua nRF24 |
| `mbmaster-v2.9.6/` | Modbus RTU | Giao thức công nghiệp qua RS485/UART |
| `net/` | TCP/IP nhỏ | Stack mạng cơ bản |

---

### 📦 `sources/libraries/` — Thư viện bên thứ 3

| Folder | Chức năng |
|--------|-----------|
| `ArduinoJson/` | Parse/tạo JSON (dùng cho cấu hình hoặc giao tiếp) |
| `nlohmann/` | Thư viện JSON C++ hiện đại |
| `QRCode/` | Tạo mã QR code hiển thị lên OLED |

---

### 🧰 `sources/common/` — Tiện ích dùng chung

| File | Chức năng |
|------|-----------|
| `screen_manager.cpp / .h` | Quản lý chuyển đổi giữa các màn hình |
| `view_render.cpp / .h` | Engine vẽ UI lên OLED |
| `view_item.cpp / .h` | Các thành phần UI cơ bản (text, icon...) |
| `xprintf.c / .h` | Hàm printf tối ưu cho embedded (thay thế printf nặng nề) |
| `cmd_line.c / .h` | Parser lệnh từ serial/shell |
| `utils.c / .h` | Các hàm tiện ích chung (math, string...) |
| `container/` | Cấu trúc dữ liệu (list, queue...) |

---

### 🔩 `sources/sys/` — Hệ thống cấp thấp

| File | Chức năng |
|------|-----------|
| `sys_boot.c / .h` | Xử lý trình tự boot, kiểm tra nguồn boot |
| `sys_dbg.c / .h` | Debug hệ thống: hard fault handler, stack dump |
| `sys_ctrl.h` | Điều khiển hệ thống: reset, sleep... |
| `sys_irq.h` | Quản lý ngắt (interrupt) |
| `sys_io.h` | Abstraction layer cho IO |

---

## 📊 Sơ đồ phân lớp tổng quát

```
┌─────────────────────────────────────────────┐
│              app/screens/                   │  ← Game UI (màn hình)
├─────────────────────────────────────────────┤
│                  app/                       │  ← Logic game, task
├──────────────┬──────────────────────────────┤
│   common/    │       networks/              │  ← Tiện ích & giao thức
├──────────────┴──────────────────────────────┤
│                   ak/                       │  ← RTOS (task, message)
├─────────────────────────────────────────────┤
│                 driver/                     │  ← Driver linh kiện
├─────────────────────────────────────────────┤
│               platform/stm32l/              │  ← Cấu hình STM32
├─────────────────────────────────────────────┤
│                   sys/                      │  ← Boot, IRQ, debug
└─────────────────────────────────────────────┘
            PHẦN CỨNG STM32L151
```

> **Quy tắc quan trọng:** Lớp trên chỉ gọi xuống lớp dưới, không gọi ngược lại. Ví dụ: `app/` gọi `driver/`, `driver/` gọi `platform/`, nhưng `platform/` không biết gì về `app/`.
