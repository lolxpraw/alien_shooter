<h1 align="center">Runtime Signal Processing</h1>

This document explains how Alien Shooter processes button input, game-loop ticks, and game object updates. Unlike multi-task designs, Alien Shooter centralizes its gameplay logic inside a single screen handler (`scr_game_handle`) owned by the display task (`AC_TASK_DISPLAY_ID`).

## I. Overview

Alien Shooter runs entirely within the display task. It uses an event-driven loop powered by periodic timers and hardware button signals.

**Key components:**
- **State Data:** All game objects (player, bullets, enemies, boss) are stored as static variables inside `scr_game.cpp`.
- **Game Tick:** The main loop runs on the `AC_DISPLAY_GAME_UPDATE` signal.
- **Interval:** The game tick interval is defined by `AC_DISPLAY_GAME_UPDATE_INTERVAL` (80 ms).
- **Input:** Hardware button callbacks post `AC_DISPLAY_BUTON_*` signals directly to the display task.

Main runtime flow:
1. `SCREEN_ENTRY` initializes the game state, plays the background music, and arms the periodic timer.
2. Button callbacks post asynchronous `PRESSED` and `RELEASED` signals. The screen updates the player position instantly and latches the state into `g_keys_held` for smooth continuous movement.
3. Every 80ms, the `AC_DISPLAY_GAME_UPDATE` signal triggers `game_update()`.
4. `game_update()` processes continuous movement, moves bullets, updates enemies/boss, resolves collisions, and checks for level transitions or game over.
5. The view renderer (`view_scr_game()`) reads the updated static variables and draws the new frame to the OLED.

---

## II. Game Start & Initialization

When the screen transitions to the game screen, the framework posts a `SCREEN_ENTRY` signal to `scr_game_handle()`.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'16px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':80,'messageFontSize':16,'noteFontSize':14,'actorFontSize':16,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':false}}}%%
sequenceDiagram
    autonumber
    participant Fw as AK Framework
    participant Scr as scr_game
    participant Tmr as Timer
    participant Bz as Buzzer

    Fw-)Scr: SCREEN_ENTRY
    activate Scr
    Note right of Scr: game_reset() clears g_score, resets g_lives to 3
    Note right of Scr: level_init(1) initializes enemies/boss based on level
    Scr->>+Bz: BUZZER_PlaySoundLoop(DOOM)
    Bz-->>-Scr: 
    Note right of Scr: clear g_keys_held and repeat ticks
    Scr->>Tmr: timer_set(AC_DISPLAY_GAME_UPDATE, 80ms, PERIODIC)
    deactivate Scr

    Note over Tmr: 80 ms later
    Tmr-)Scr: AC_DISPLAY_GAME_UPDATE (periodic tick)
```

<p align="center"><strong><em>Figure 1:</em></strong> Game start sequence logic</p>

---

## III. Gameplay Loop & Input Handling

Input handling in Alien Shooter uses a hybrid approach:
- **Instant Response:** Pressing a button instantly applies movement or shoots a bullet.
- **Continuous Response:** Holding a button latches a bit in `g_keys_held`. The periodic `game_update()` reads this mask and applies repeated movement (`KEY_REPEAT_MOVE_TICKS`) or continuous shooting (`KEY_REPEAT_SHOOT_TICKS`) if enough ticks have passed.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'16px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':80,'messageFontSize':16,'noteFontSize':14,'actorFontSize':16,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':false}}}%%
sequenceDiagram
    autonumber
    actor Btn as Button
    participant Scr as scr_game
    participant Tmr as Timer
    
    Note over Btn,Scr: Asynchronous button press
    Btn-)Scr: AC_DISPLAY_BUTON_UP_PRESSED (Left)
    activate Scr
    Note right of Scr: px -= PLAYER_SPEED (Instant move)<br/>g_keys_held |= KEY_LEFT
    deactivate Scr
    
    Note over Tmr,Scr: Periodic Tick (AC_DISPLAY_GAME_UPDATE)
    Tmr-)Scr: AC_DISPLAY_GAME_UPDATE
    activate Scr
    Note right of Scr: game_update() begins
    
    opt g_keys_held & KEY_LEFT
        Note right of Scr: Check g_key_move_tick delay
        Note right of Scr: px -= PLAYER_SPEED (Continuous move)
    end
    
    Note right of Scr: 1. Move Player Bullets up<br/>2. Move Enemy Bullets down<br/>3. Move Enemies (X offset, bounce, drop Y)<br/>4. Enemy/Boss fires new bullet
    deactivate Scr
    
    Note over Btn,Scr: Asynchronous button release
    Btn-)Scr: AC_DISPLAY_BUTON_UP_RELEASED (Left)
    activate Scr
    Note right of Scr: g_keys_held &= ~KEY_LEFT
    deactivate Scr
```

<p align="center"><strong><em>Figure 2:</em></strong> Input and gameplay loop logic</p>

---

## IV. Collision, Level Clear, and Game Over

All object state updates are processed synchronously within `game_update()`.
- **Enemy hit:** When a player bullet intersects an enemy, `hit_enemy()` hides the enemy, increases the score, and plays a sound.
- **Player hit:** When an enemy bullet hits the player, `g_lives` is decremented. If `g_lives` drops to 0, `g_state` switches to `ST_GAME_OVER`.
- **Level Clear:** Checked at the end of `game_update()`. If `e_alive == 0`, `g_state` switches to `ST_LEVEL_CLEAR`, granting a score bonus and starting a countdown timer before loading the next level.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'16px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':80,'messageFontSize':16,'noteFontSize':14,'actorFontSize':16,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':false}}}%%
sequenceDiagram
    autonumber
    participant Tmr as Timer
    participant Scr as scr_game (game_update)
    participant Bz as Buzzer

    Tmr-)Scr: AC_DISPLAY_GAME_UPDATE
    activate Scr
    
    Note right of Scr: Check Player Bullets vs Enemies
    opt hit_enemy(pb.x, pb.y) returns true
        Note right of Scr: enemy visible = false<br/>e_alive--<br/>g_score += SCORE_KILL
        Scr->>+Bz: BUZZER_PlaySound(PEW)
        Bz-->>-Scr: 
    end
    
    Note right of Scr: Check Enemy Bullets vs Player
    opt eb hits player bounds
        alt g_lives > 1
            Note right of Scr: g_lives--<br/>clear all enemy bullets
            Scr->>+Bz: BUZZER_PlaySound(BANG)
            Bz-->>-Scr: 
        else g_lives == 1
            Note right of Scr: g_state = ST_GAME_OVER
            Scr->>+Bz: BUZZER_StopLoop()<br/>BUZZER_PlaySound(GAMEOVER)
            Bz-->>-Scr: 
            Note right of Scr: return early
        end
    end
    
    Note right of Scr: Check Level Clear
    opt e_alive == 0
        Note right of Scr: g_score += SCORE_LEVEL<br/>g_state = ST_LEVEL_CLEAR<br/>clr_ticks = 20
        Scr->>+Bz: BUZZER_PlaySound(WELCOME)
        Bz-->>-Scr: 
    end
    
    deactivate Scr
```

<p align="center"><strong><em>Figure 3:</em></strong> Collision and state transition logic</p>

---

## V. Code References

| Logic Area | Location in `scr_game.cpp` |
|---|---|
| Screen Handler (Signal Routing) | `scr_game_handle()` |
| Game Update Loop | `game_update()` |
| Initialization / Next Level | `level_init()` |
| Player Bullet Firing | `do_shoot()` |
| Enemy Bullet Firing | `enemy_shoot_one()`, `boss_shoot()` |
| Collision Detection | `hit_enemy()` |
| Rendering (OLED Drawing) | `view_scr_game()`, `draw_enemy()`, `draw_boss()` |

All game logic operates inside `application/sources/app/screens/scr_game.cpp`. No custom RTOS tasks are needed for individual game objects.
