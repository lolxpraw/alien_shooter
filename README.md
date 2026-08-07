# 📁 Project Directory Structure — `alien_shooter`

This is a game project running on the **STM32L1** microcontroller (AK Embedded Base Kit), built on top of the **AK Framework** — a lightweight RTOS with a **task + message** event-driven programming model.

---

## 🗂️ Overview

```
test_game/
├── hardware/        ← Hardware documentation (schematics, images, BOM)
├── boot/            ← Bootloader program (OTA firmware update)
├── application/     ← Main application (game + all logic)
└── .vscode/         ← Editor configuration (does not affect code)
```

---

## 🔧 `hardware/` — Hardware Documentation

> Contains no source code — only hardware design documents.

| Folder | Contents |
|--------|----------|
| `schematic/` | Circuit schematic files (`.pdf`) — versions 2 and 3 of the AK Base Kit |
| `images/` | Photos of the physical PCB board |
| `board-assembly/` | Component assembly documentation |
| `manufacturing/` | PCB manufacturing files (Gerber, BOM, etc.) |
| `bin/` | Pre-built binary firmware samples |

---

## 🥾 `boot/` — Bootloader

> The **first program** that runs when the board is powered on. Its main job is to check if a new firmware is available for OTA update; if not, it jumps to the `application`.

### `boot/sources/` contains:

| Folder/File | Function |
|-------------|----------|
| `app/` | Bootloader business logic (check flash, decide whether to jump to app) |
| `driver/` | Minimal hardware drivers required for boot (flash, UART, etc.) |
| `platform/` | STM32 hardware configuration for the bootloader |
| `sys/` | Lowest-level system: chip initialization, debug |
| `common/` | Shared utilities |
| `Makefile` | Build script for the bootloader |
| `stm32l_init.gdb` | GDB script for flashing/debugging via ST-Link |

---

## 📱 `application/` — Main Application (Game)

> This is the **most important part** — it contains all game code and system logic.

```
application/
├── Makefile
├── stm32l_init.gdb
├── build_ak-base-kit-stm32l151-application/   ← Output directory after build
└── sources/                                    ← All source code
    ├── ak/         ← AK Framework (lightweight RTOS)
    ├── app/        ← Application / game logic
    ├── common/     ← Shared utilities
    ├── driver/     ← Peripheral drivers (display, buzzer, buttons, etc.)
    ├── libraries/  ← Third-party libraries
    ├── networks/   ← Network protocols (Zigbee, RF, Modbus)
    ├── platform/   ← STM32 hardware configuration
    └── sys/        ← Low-level system layer
```

---

### ⚙️ `sources/ak/` — AK Framework (RTOS)

> The **"heart"** of the system. AK is a custom-built mini-RTOS that allows multiple **tasks** to run concurrently using an **event-driven** model (receive and process messages).

| Folder | Contents |
|--------|----------|
| `inc/` | Header files: Task, Message, Timer, Signal definitions, etc. |
| `src/` | AK kernel source code |
| `doc/` | AK framework documentation |
| `ak.cfg.mk` | Feature configuration file for AK at build time |

---

### 🎮 `sources/app/` — Application Logic (Main Game)

> This is where **all game code and task management** lives.

#### Key Files:

| File | Function |
|------|----------|
| `app.cpp / app.h` | Application entry point, initializes all tasks |
| `task_list.cpp / .h` | **Complete task registry** (task IDs, priorities) |
| `task_display.cpp` | Task managing OLED screen rendering |
| `task_system.cpp` | System task (reset, watchdog, etc.) |
| `task_life.cpp` | Heartbeat task — blinks LED to indicate the board is alive |
| `task_shell.cpp` | Debug shell task over UART (command-line interface) |
| `task_fw.cpp` | Firmware update task (OTA) |
| `task_zigbee.cpp` | Zigbee wireless communication task |
| `app_data.cpp / .h` | Global application data |
| `app_bsp.cpp / .h` | Board Support Package — hardware initialization for the app |
| `shell.cpp` | Implementation of serial debug commands |

#### Folder `screens/` — Display Screens:

> Each `scr_*.cpp` file represents **one screen** on the OLED display.

| File | Screen |
|------|--------|
| `scr_startup.cpp` | Startup screen (logo on power-on) |
| `scr_welcome.cpp` | Welcome screen |
| `scr_idle.cpp` | Idle/standby screen |
| `scr_game.cpp` | **Main game screen** (Space Invaders!) |
| `scr_image.cpp` | Bitmap image display screen |
| `scr_qrcode.cpp` | QR code display screen |
| `screens_bitmap.cpp` | Bitmap image storage used in the game |
| `screens.h` | Lists all screens, used by `screen_manager` for navigation |

---

### 🛠️ `sources/driver/` — Hardware Drivers

> Each **sub-folder** is a driver for a specific component on the board.

| Folder | Device | Function |
|--------|--------|----------|
| `Adafruit_oled_drv/` | SSD1306 OLED display | Draw pixels, text, and shapes on screen |
| `buzzer/` | Piezo buzzer | Play music and game sound effects |
| `button/` | Push buttons | Read button state with debouncing |
| `led/` | LEDs | Control LED on/off |
| `gpio/` | General GPIO | Configure and read/write GPIO pins |
| `eeprom/` | Internal EEPROM | Persistent data storage (survives power-off) |
| `flash/` | Internal Flash | Store OTA firmware to flash memory |
| `nRF24/` | nRF24L01 RF module | Wireless data transmission and reception |
| `AsyncDelay/` | Non-blocking timer | Create delays without blocking other tasks |

#### `buzzer/` Details:

| File | Function |
|------|----------|
| `buzzer.c / .h` | PWM hardware control for sound generation |
| `buzzer_music.c / .h` | Music library (Pirates of the Caribbean, Mario, etc.) with note-sequence playback |

---

### 🔌 `sources/platform/stm32l/` — STM32 Configuration

> Middleware layer between drivers and the actual STM32 hardware.

| File | Function |
|------|----------|
| `io_cfg.c / .h` | **All IO pin configuration** (SPI, I2C, UART, PWM, etc.) |
| `sys_cfg.c / .h` | System clock, timer, and interrupt configuration |
| `system.c / .h` | STM32 chip initialization functions |
| `system_stm32l1xx.c` | ST's standard init file for STM32L1 |
| `platform.c / .h` | Platform abstraction layer |
| `ak.ld` | Linker script — defines Flash/RAM memory regions |
| `mini_cpp.cpp` | Minimal C++ support for bare-metal (new/delete operators) |
| `Libraries/` | STMicroelectronics HAL/SPL library |
| `arduino/` | Arduino compatibility layer (for drivers written in Arduino style) |

---

### 🌐 `sources/networks/` — Communication Protocols

| Folder | Protocol | Function |
|--------|----------|----------|
| `ArduinoZigBee/` | Zigbee | Wireless mesh networking |
| `rf_protocols/` | Custom RF | RF protocol over nRF24 |
| `mbmaster-v2.9.6/` | Modbus RTU | Industrial protocol over RS485/UART |
| `net/` | Mini TCP/IP | Basic networking stack |

---

### 📦 `sources/libraries/` — Third-Party Libraries

| Folder | Function |
|--------|----------|
| `ArduinoJson/` | Parse/generate JSON (for configuration or communication) |
| `nlohmann/` | Modern C++ JSON library |
| `QRCode/` | Generate QR codes to display on the OLED |

---

### 🧰 `sources/common/` — Shared Utilities

| File | Function |
|------|----------|
| `screen_manager.cpp / .h` | Manage transitions between screens |
| `view_render.cpp / .h` | UI rendering engine for OLED |
| `view_item.cpp / .h` | Basic UI components (text, icons, etc.) |
| `xprintf.c / .h` | Lightweight printf for embedded systems (replaces heavy standard printf) |
| `cmd_line.c / .h` | Command parser for serial/shell input |
| `utils.c / .h` | General-purpose utility functions (math, string, etc.) |
| `container/` | Data structures (list, queue, etc.) |

---

### 🔩 `sources/sys/` — Low-Level System

| File | Function |
|------|----------|
| `sys_boot.c / .h` | Boot sequence handling, boot source detection |
| `sys_dbg.c / .h` | System debug: hard fault handler, stack dump |
| `sys_ctrl.h` | System control: reset, sleep, etc. |
| `sys_irq.h` | Interrupt management |
| `sys_io.h` | IO abstraction layer |

---

## 📊 Architecture Layer Diagram

```
┌─────────────────────────────────────────────┐
│              app/screens/                   │  ← Game UI (display screens)
├─────────────────────────────────────────────┤
│                  app/                       │  ← Game logic, tasks
├──────────────┬──────────────────────────────┤
│   common/    │       networks/              │  ← Utilities & protocols
├──────────────┴──────────────────────────────┤
│                   ak/                       │  ← RTOS (task, message)
├─────────────────────────────────────────────┤
│                 driver/                     │  ← Component drivers
├─────────────────────────────────────────────┤
│               platform/stm32l/              │  ← STM32 configuration
├─────────────────────────────────────────────┤
│                   sys/                      │  ← Boot, IRQ, debug
└─────────────────────────────────────────────┘
                STM32L151 HARDWARE
```

> **Key rule:** Upper layers only call down to lower layers, never the reverse. For example: `app/` calls `driver/`, `driver/` calls `platform/`, but `platform/` has no knowledge of `app/`.
