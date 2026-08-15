<h1 align="center"> Alien Shooter — Space Invaders on STM32</h1>

<p align="center">
  <em>A classic Space Invaders game running on the <strong>STM32L151</strong> microcontroller (AK Embedded Base Kit), featuring boss fights, progressive difficulty, sound effects, and background music — all rendered on a 128×64 OLED display.</em>
</p>

<p align="center">
  <a href="https://github.com/lolxpraw/alien_shooter">
    <img src="https://img.shields.io/badge/Platform-STM32L151-blue?style=for-the-badge&logo=stmicroelectronics&logoColor=white"/>
  </a>
  <a href="https://github.com/lolxpraw/alien_shooter/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge"/>
  </a>
  <a href="#">
    <img src="https://img.shields.io/badge/Display-SSD1306_OLED-orange?style=for-the-badge"/>
  </a>
  <a href="#">
    <img src="https://img.shields.io/badge/Framework-AK_RTOS-purple?style=for-the-badge"/>
  </a>
</p>

<!-- TODO: Add gameplay screenshot/gif here -->
<!-- <p align="center">
  <img width="600" alt="gameplay" src="path/to/gameplay_screenshot.png" />
</p> -->

<img width="1024" height="512" alt="image" src="https://github.com/user-attachments/assets/97af589b-bce3-4af5-8815-396e83e53c0d" />

---

## Demo


https://github.com/user-attachments/assets/5f9ddeb5-3e8e-43e3-b2de-e2846516fd87

## Hardware

- This kit integrates 1.54" Oled LCD, 3 push buttons, and 1 buzzer, which would be sufficient to create a small video game with an event driven paradigm.
- It also includes RS485, Qwiic Connect System, and Grove Ecosystems, suitable for prototyping other practical applications in embedded systems.

[<img src="hardware/images/ak-embedded-base-kit-version-3.jpg" width="480"/>](<https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu>)


## Memory map

AK base kit uses the following memory map to run its application code

- [ 0x08000000 ] : **Boot** [[ak-base-kit-stm32l151-boot.bin]](https://github.com/ak-embedded-software/ak-base-kit-stm32l151/blob/main/hardware/bin/ak-base-kit-stm32l151-boot.bin)
- [ 0x08002000 ] : **BSF** [ Memory for data sharing between Boot and Application ]
- [ 0x08003000 ] : **Application** [[ak-base-kit-stm32l151-application.bin]](https://github.com/ak-embedded-software/ak-base-kit-stm32l151/blob/main/hardware/bin/ak-base-kit-stm32l151-application.bin)                                             |

>**Note:** After loading the boot and application firmware, you can use [AK - Flash](https://github.com/ak-embedded-software/ak-flash), a CLI to work with the AK base kit, to load the application directly through the kit's USB port. Once installed, the following command will flash user's defined code into the kit's application's memory region.

```sh
ak_flash /dev/ttyUSB0 ak-base-kit-stm32l151-application.bin 0x08003000
```
## About the Game

Alien Shooter is a Space Invaders-style game developed for the AK Embedded Base Kit STM32L151. The game features a player-controlled ship at the bottom of a 128×64 OLED screen, battling waves of descending aliens and periodic boss encounters.

Built on top of the AK Framework — a lightweight custom RTOS with a task + message event-driven programming model — this project demonstrates real-time embedded game development on resource-constrained hardware.

---

## Game Features

| Feature | Description |
|---------|-------------|
|  **Progressive Difficulty** | Enemy speed, bullet count, and firing rate increase each level |
| **3 Enemy Types** | Top row (^^ shape), mid row (armed rect), bottom row (crab) — each with unique animations |
| **Boss Fights** | Every 3rd level features a large UFO boss with HP bar and 3 attack patterns |
| **3 Lives System** | Player has 3 lives; HUD displays remaining lives in real-time |
| **Dual-Bullet Mode** | From Level 7+, the player fires alternating dual bullets |
| **Background Music** | "Doom OST" plays during gameplay with seamless loop |
| **Sound Effects** | Distinct sounds for shooting (PEW), enemy hit (BANG), game over, and level clear |
| **Score & Level HUD** | Real-time score, lives, and level display at the top of the screen |
| **Randomized Spawns** | 70% spawn chance per grid slot creates unique enemy formations each level |

---

## Controls

| Button | Action |
|--------|--------|
| **UP** | Move ship **LEFT** |
| **DOWN** | Move ship **RIGHT** |
| **MODE** | **SHOOT** |

> **Tip:** Hold any button for auto-repeat! Movement repeats every ~160ms and shooting every ~400ms.

---

## Project Structure

```
alien_shooter/
├── hardware/          ← Hardware documentation (schematics, images, BOM)
├── boot/              ← Bootloader program (OTA firmware update)
├── application/       ← Main application (game + all logic)
│   ├── Makefile
│   ├── stm32l_init.gdb
│   └── sources/
│       ├── ak/        ← AK Framework (lightweight RTOS)
│       ├── app/       ← Application / game logic
│       │   ├── screens/
│       │   │   ├── scr_game.cpp      ← Main game (Space Invaders)
│       │   │   ├── scr_startup.cpp   ← Boot logo screen
│       │   │   ├── scr_welcome.cpp   ← Welcome screen
│       │   │   ├── scr_idle.cpp      ← Idle/standby screen
│       │   │   ├── scr_image.cpp     ← Bitmap image display
│       │   │   ├── scr_qrcode.cpp    ← QR code display
│       │   │   └── screens_bitmap.cpp← Bitmap data storage
│       │   ├── task_display.cpp      ← OLED display task
│       │   ├── task_system.cpp       ← System task (reset, watchdog)
│       │   ├── task_life.cpp         ← Heartbeat LED task
│       │   ├── task_shell.cpp        ← UART debug shell
│       │   └── task_list.cpp         ← Task registry (IDs, priorities)
│       ├── common/    ← Shared utilities (screen manager, renderer)
│       ├── driver/    ← Peripheral drivers
│       │   ├── Adafruit_oled_drv/    ← SSD1306 OLED driver
│       │   ├── buzzer/               ← Buzzer + music library
│       │   ├── button/               ← Button input with debounce
│       │   ├── led/                  ← LED control
│       │   └── ...
│       ├── libraries/ ← Third-party (ArduinoJson, QRCode)
│       ├── networks/  ← Communication (Zigbee, RF, Modbus)
│       ├── platform/  ← STM32 HAL configuration
│       └── sys/       ← Low-level system (boot, IRQ, debug)
├── docs/              ← Project documentation
├── LICENSE            ← MIT License
└── .vscode/           ← Editor configuration
```

---

## Getting Started

### Prerequisites

- **OS:** Ubuntu / Linux
- **Toolchain:** `arm-none-eabi-gcc` (ARM cross-compiler)
- **Debugger:** ST-Link V2 + `openocd` or `st-flash`
- **Editor:** VSCode (recommended)

> For detailed environment setup, follow the official guide:
> **[AK Embedded Base Kit STM32L151 — Getting Started](https://epcb.vn/blogs/ak-embedded-software/ak-embedded-base-kit-stm32l151-getting-started)**

### Clone & Build

```bash
# 1. Clone the repository
git clone https://github.com/lolxpraw/alien_shooter.git

# 2. Navigate to the application directory
cd alien_shooter/application

# 3. Build the firmware
make all
```

<!-- TODO: Add build output screenshot here -->
<!-- <p align="center">
  <img width="800" alt="build-output" src="path/to/build_screenshot.png" />
</p> -->

### Flash to Board

```bash
# Connect ST-Link to the board, then run:
make flash
```


## Architecture Overview

```
┌─────────────────────────────────────────────┐
│            app/screens/                     │  ← Game UI (OLED screens)
│   scr_game.cpp  scr_idle.cpp  scr_startup   │
├─────────────────────────────────────────────┤
│                  app/                       │  ← Game logic & tasks
│   task_display  task_system  task_life       │
├──────────────┬──────────────────────────────┤
│   common/    │       networks/              │  ← Utilities & protocols
│ screen_mgr   │   Zigbee, RF, Modbus         │
├──────────────┴──────────────────────────────┤
│                   ak/                       │  ← RTOS (task + message)
├─────────────────────────────────────────────┤
│                 driver/                     │  ← OLED, buzzer, buttons
├─────────────────────────────────────────────┤
│               platform/stm32l/              │  ← STM32 HAL config
├─────────────────────────────────────────────┤
│                   sys/                      │  ← Boot, IRQ, debug
└─────────────────────────────────────────────┘
                STM32L151 HARDWARE
```

---

## Game Mechanics

### Normal Levels

- **6×3 enemy grid** with randomized spawn (70% per slot)
- Enemies march left/right and drop down when hitting screen edges
- If enemies reach the ground → **Game Over**
- Enemy speed, bullet count, and fire rate scale with level:

| Level | Move Speed | Fire Interval | Max Bullets | Bullet Speed |
|-------|-----------|---------------|-------------|--------------|
| 1     | 8 ticks   | 16 ticks      | 1           | 2 px/tick    |
| 2     | 6 ticks   | 11 ticks      | 2           | 3 px/tick    |
| 3     | 4 ticks   | 7 ticks       | 3           | 4 px/tick    |
| 4     | 2 ticks   | 5 ticks       | 4           | 5 px/tick    |
| 5+    | 2 ticks   | 3 ticks       | 5           | 6 px/tick    |

<img width="512" height="256" alt="lv1" src="https://github.com/user-attachments/assets/e54f4dbd-fe3c-4328-be93-cae8419795d5" />


### Boss Levels (Every 3rd Level)

- A large **UFO boss** appears with an HP bar (HP = level × 10)
- 3 attack patterns that switch randomly:
  - **Normal** — random bullet position
  - **Aimed** — bullet fired directly at the player
  - **Spread** — two bullets from both edges
- Boss randomly changes direction for unpredictable movement

<img width="512" height="256" alt="boss" src="https://github.com/user-attachments/assets/916c9b6a-9c91-461c-9357-3b5b1f04e4b0" />


### Scoring

| Event | Points |
|-------|--------|
| Kill enemy / Hit boss | +10 |
| Clear level | +50 |

<img width="512" height="256" alt="lv_clear" src="https://github.com/user-attachments/assets/a0e7aa5a-5775-4d34-bc79-463f67bcdb4f" />

---

## Sound & Music

The game features a rich audio experience powered by the **piezo buzzer** driver:

| Sound | Trigger |
|-------|---------|
| Doom | Background music (loops during gameplay) |
| PEW | Player bullet hits an enemy |
| BANG | Player gets hit (loses a life) |
| Game Over | All lives lost |
| Welcome | Level cleared |

---

## Documentation

Detailed guides are available in the `docs/` directory:

| Document | Content |
|----------|---------|
| [runtime_signal.md](docs/runtime_signal.md) | Runtime signal processing logic and diagrams |
| [game_object.md](docs/game_object.md) | Game object sequences and behaviors |

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

```
Copyright (c) 2022 AK Foundation
```
