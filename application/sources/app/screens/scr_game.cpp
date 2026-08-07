/*
 * scr_game.cpp — Space Invaders  (OLED 124x60)
 *
 * [UP]   = Di chuyen TRAI
 * [DOWN] = Di chuyen PHAI
 * [MODE] = Ban dan
 */
#include "scr_game.h"

/* ========== CONSTANTS ========== */
#define HUD_H           8
#define GROUND_Y        54
#define PLAY_TOP        (HUD_H + 1)

#define PLAYER_SPEED    5
#define PLAYER_X_MIN    5
#define PLAYER_X_MAX    (LCD_WIDTH - 6)

#define MAX_P_BULLETS   2
#define P_BULLET_SPD    7

#define ENEMY_COLS      6
#define ENEMY_ROWS      3
#define ENEMY_W         7
#define ENEMY_H         3
#define ENEMY_X_STEP    13
#define ENEMY_Y_STEP    8
#define ENEMY_GRID_W    (ENEMY_COLS * ENEMY_X_STEP - (ENEMY_X_STEP - ENEMY_W))
#define ENEMY_START_X   ((LCD_WIDTH - ENEMY_GRID_W) / 2)
#define ENEMY_START_Y   (PLAY_TOP + 2)
#define ENEMY_MOVE_PX   2
#define ENEMY_DROP_PX   4

#define MAX_E_BULLETS   5           /* pool tối đa - không thay đổi (array size) */
#define E_SHOOT_BASE    16          /* interval bắn base (ticks) */

#define SCORE_KILL      10
#define SCORE_LEVEL     50

#define SPD_LV1         8
#define SPD_LV2         6
#define SPD_LV3         4
#define SPD_LV4         2

/* ========== TYPES ========== */
typedef enum { ST_PLAYING, ST_LEVEL_CLEAR, ST_GAME_OVER } game_st_t;

typedef struct { int16_t x, y; bool on; } bullet_t;

/* ========== STATE ========== */
static game_st_t  g_state;
static uint16_t   g_score;
static uint8_t    g_level;
static uint8_t    g_lives;

static int16_t    px;                           /* player x */
static bullet_t   pb[MAX_P_BULLETS];            /* player bullets */
static bool       p_alt_gun;

static bool       en[ENEMY_ROWS][ENEMY_COLS];   /* enemy alive */
static int16_t    e_ox, e_oy;                   /* enemy grid offset */
static int8_t     e_dir;                        /* 1=right, -1=left */
static uint8_t    e_alive;
static uint8_t    e_move_ticks, e_tick;
static bool       e_anim;

static bullet_t   eb[MAX_E_BULLETS];            /* enemy bullets */
static uint8_t    e_shoot_iv, e_shoot_t;
static uint8_t    e_max_bullets;                /* số đạn đồng thời tối đa theo level */
static uint8_t    e_bullet_spd;                 /* tốc độ đạn kẻ địch theo level */

/* ========== BOSS STATE ========== */
static bool       is_boss_level;
static int16_t    b_x, b_y;
static int16_t    b_target_x;
static int16_t    b_hp, b_max_hp;
static int8_t     b_dir;
static uint8_t    b_tick;
static uint8_t    b_move_ticks;
static uint8_t    b_shoot_iv, b_shoot_t;
static uint8_t    b_state; /* 0=normal, 1=rapid, 2=spread */
#define BOSS_W    15
#define BOSS_H    8

/* ========== LCG PSEUDO-RANDOM (nhẹ, không cần stdlib) ========== */
static uint16_t rng_seed = 0xA5B3;
static uint8_t rng8() {
    rng_seed = (uint16_t)(rng_seed * 6364u + 1u);
    return (uint8_t)(rng_seed >> 7);
}

static uint8_t    clr_ticks;                    /* level clear countdown */

/* ========== HELD KEY STATE ========== */
/* Bit flags: bit0=LEFT(UP), bit1=RIGHT(DOWN), bit2=SHOOT(MODE) */
#define KEY_LEFT    (1u << 0)
#define KEY_RIGHT   (1u << 1)
#define KEY_SHOOT   (1u << 2)
static uint8_t g_keys_held = 0;                 /* bitmask phím đang giữ */
#define KEY_REPEAT_MOVE_TICKS  2   /* lặp di chuyển mỗi N game tick (160ms) */
#define KEY_REPEAT_SHOOT_TICKS 5   /* lặp bắn mỗi N game tick (400ms) */
static uint8_t g_key_move_tick  = 0;
static uint8_t g_key_shoot_tick = 0;

/* ========== FRAMEWORK ========== */
static void view_scr_game();
static void do_shoot();

view_dynamic_t dyn_view_game = {{ .item_type = ITEM_TYPE_DYNAMIC }, view_scr_game};
view_screen_t  scr_game      = {&dyn_view_game, ITEM_NULL, ITEM_NULL, .focus_item=0};

/* ========== DRAW HELPERS ========== */

static void draw_enemy(int16_t x, int16_t y, uint8_t row, bool anim) {
    if (row == 0) {
        /* top row: ^^ shape */
        view_render.drawPixel(x+1, y+!anim, WHITE);
        view_render.drawPixel(x+5, y+!anim, WHITE);
        view_render.drawLine(x, y+1, x+6, y+1, WHITE);
        view_render.drawPixel(x+2, y+2, WHITE);
        view_render.drawPixel(x+4, y+2, WHITE);
    } else if (row == 1) {
        /* mid row: rect with side arms */
        view_render.drawRect(x+1, y, 5, 3, WHITE);
        view_render.drawPixel(anim ? x : x+6, y+1, WHITE);
        view_render.drawPixel(anim ? x+6 : x, y+1, WHITE);
    } else {
        /* bottom row: crab */
        view_render.fillRect(x+1, y, 5, 2, WHITE);
        view_render.drawPixel(x+anim, y+2,   WHITE);
        view_render.drawPixel(x+6-anim, y+2, WHITE);
    }
}

static void draw_boss(int16_t x, int16_t y, bool anim) {
    /* Large UFO shape */
    view_render.drawRect(x+3, y, 9, 3, WHITE);
    view_render.drawLine(x+1, y+3, x+13, y+3, WHITE);
    view_render.drawLine(x, y+4, x+14, y+4, WHITE);
    view_render.drawPixel(x+2, y+5, WHITE);
    view_render.drawPixel(x+12, y+5, WHITE);
    
    if (anim) {
        view_render.drawPixel(x+4, y+6, WHITE);
        view_render.drawPixel(x+10, y+6, WHITE);
    } else {
        view_render.drawPixel(x+5, y+6, WHITE);
        view_render.drawPixel(x+9, y+6, WHITE);
    }
}

static void draw_player(int16_t cx) {
    /* Vẽ người chơi giống hình dáng của enemy (hàng trên cùng) */
    draw_enemy(cx - 3, GROUND_Y - 4, 0, false);
}

/* ========== VIEW ========== */

static void view_scr_game() {
    view_render.clear();

    /* --- GAME OVER --- */
    if (g_state == ST_GAME_OVER) {
        view_render.setTextColor(WHITE);
        view_render.setTextSize(1);
        view_render.setCursor(34, 8);  view_render.print("GAME OVER");
        view_render.setCursor(20, 22); view_render.print("SCORE: "); view_render.print((int)g_score);
        view_render.setCursor(20, 34); view_render.print("LEVEL: "); view_render.print((int)g_level);
        view_render.setCursor(12, 48); view_render.print("PRESS ANY KEY");
        return;
    }

    /* --- LEVEL CLEAR --- */
    if (g_state == ST_LEVEL_CLEAR) {
        view_render.setTextColor(WHITE);
        view_render.setTextSize(1);
        view_render.setCursor(28, 12); view_render.print("LEVEL CLEAR!");
        view_render.setCursor(20, 26); view_render.print("SCORE: "); view_render.print((int)g_score);
        view_render.setCursor(20, 38); view_render.print("NEXT LV: "); view_render.print((int)(g_level+1));
        return;
    }

    /* --- HUD --- */
    view_render.setTextColor(WHITE);
    view_render.setTextSize(1);
    view_render.setCursor(0, 0);  view_render.print("L:"); view_render.print((int)g_lives);
    view_render.setCursor(24, 0); view_render.print((int)g_score);
    view_render.setCursor(96, 0); view_render.print("LV:"); view_render.print((int)g_level);
    
    if (is_boss_level) {
        /* Draw boss HP bar at the top center */
        int16_t hp_w = (38 * b_hp) / b_max_hp;
        if (hp_w < 0) hp_w = 0;
        view_render.drawRect(52, 1, 40, 6, WHITE);
        view_render.fillRect(53, 2, hp_w, 4, WHITE);
    }

    view_render.drawLine(0, HUD_H-1, LCD_WIDTH-1, HUD_H-1, WHITE);

    /* --- ground & player --- */
    view_render.drawLine(0, GROUND_Y, LCD_WIDTH-1, GROUND_Y, WHITE);
    draw_player(px);

    /* --- player bullets --- */
    for (uint8_t i = 0; i < MAX_P_BULLETS; i++) {
        if (!pb[i].on) continue;
        view_render.drawLine(pb[i].x, pb[i].y, pb[i].x, pb[i].y+2, WHITE);
    }

    /* --- enemies / boss --- */
    if (is_boss_level) {
        draw_boss(b_x, b_y, e_anim);
    } else {
        for (uint8_t r = 0; r < ENEMY_ROWS; r++) {
            for (uint8_t c = 0; c < ENEMY_COLS; c++) {
                if (!en[r][c]) continue;
                int16_t ex = ENEMY_START_X + e_ox + (int16_t)c * ENEMY_X_STEP;
                int16_t ey = ENEMY_START_Y + e_oy + (int16_t)r * ENEMY_Y_STEP;
                draw_enemy(ex, ey, r, e_anim);
            }
        }
    }

    /* --- enemy bullets --- */
    for (uint8_t i = 0; i < MAX_E_BULLETS; i++) {
        if (!eb[i].on) continue;
        view_render.drawPixel(eb[i].x, eb[i].y, WHITE);
    }
}

/* ========== GAME LOGIC ========== */

static void level_init(uint8_t lv) {
    g_level = lv;
    g_state = ST_PLAYING;
    e_alive = 0;
    is_boss_level = (lv > 0 && lv % 3 == 0);
    
    if (is_boss_level) {
        b_max_hp = lv * 10;
        b_hp = b_max_hp;
        b_x = (LCD_WIDTH - BOSS_W) / 2;
        b_target_x = b_x;
        b_y = PLAY_TOP + 2;
        b_dir = 1;
        b_tick = 0;
        b_move_ticks = (lv <= 3) ? 4 : 2;
        b_shoot_iv = (lv <= 3) ? 10 : 6;
        b_shoot_t = 0;
        b_state = 0;
        e_alive = 1;
    } else {
        for (uint8_t r = 0; r < ENEMY_ROWS; r++) {
            for (uint8_t c = 0; c < ENEMY_COLS; c++) {
                bool spawn = (rng8() % 100) < 70; /* 70% chance to spawn */
                en[r][c] = spawn;
                if (spawn) e_alive++;
            }
        }
        if (e_alive == 0) {
            en[0][0] = true;
            e_alive = 1;
        }
    }
    e_ox = 0; e_oy = 0; e_dir = 1; e_anim = false; e_tick = 0;
    /* tốc độ di chuyển kẻ địch */
    if (lv <= 1)     e_move_ticks = SPD_LV1;
    else if (lv==2)  e_move_ticks = SPD_LV2;
    else if (lv==3)  e_move_ticks = SPD_LV3;
    else             e_move_ticks = SPD_LV4;
    /* interval bắn: càng cao level càng ngắn (bắn nhanh hơn) */
    if (lv <= 1)     e_shoot_iv = 16;
    else if (lv==2)  e_shoot_iv = 11;
    else if (lv==3)  e_shoot_iv = 7;
    else if (lv==4)  e_shoot_iv = 5;
    else             e_shoot_iv = 3;
    /* số đạn đồng thời theo level (1 → 2 → 3 → 4 → 5) */
    e_max_bullets = (lv < MAX_E_BULLETS) ? lv : MAX_E_BULLETS;
    /* tốc độ đạn kẻ địch theo level */
    e_bullet_spd  = (lv <= 1) ? 2 :
                    (lv == 2) ? 3 :
                    (lv == 3) ? 4 :
                    (lv == 4) ? 5 : 6;
    e_shoot_t  = 0;
    for (uint8_t i = 0; i < MAX_E_BULLETS; i++) eb[i].on = false;
    for (uint8_t i = 0; i < MAX_P_BULLETS; i++) pb[i].on = false;
    px = LCD_WIDTH / 2;
    /* Phát lại nhạc mỗi khi khởi tạo/sang level mới */
    BUZZER_PlaySoundLoop(BUZZER_SOUND_PIRATES);
}

static void game_reset() { 
    g_score = 0; 
    g_lives = 3;
    p_alt_gun = false;
    level_init(1); 
}

static void enemy_bounds(int16_t &left, int16_t &right, int16_t &bot) {
    left=1000; right=-1000; bot=-1000;
    for (uint8_t r=0; r<ENEMY_ROWS; r++) {
        for (uint8_t c=0; c<ENEMY_COLS; c++) {
            if (!en[r][c]) continue;
            int16_t ex = ENEMY_START_X + e_ox + (int16_t)c * ENEMY_X_STEP;
            int16_t ey = ENEMY_START_Y + e_oy + (int16_t)r * ENEMY_Y_STEP;
            if (ex < left)              left  = ex;
            if (ex + ENEMY_W > right)   right = ex + ENEMY_W;
            if (ey + ENEMY_H > bot)     bot   = ey + ENEMY_H;
        }
    }
}

static void enemy_shoot_one() {
    /* Đếm số đạn đang hoạt động */
    uint8_t active = 0;
    for (uint8_t i = 0; i < MAX_E_BULLETS; i++) if (eb[i].on) active++;
    if (active >= e_max_bullets) return;  /* đã đạt giới hạn level */

    /* Chọn cột bắn ngẫu nhiên */
    uint8_t start = rng8() % ENEMY_COLS;
    for (uint8_t ci = 0; ci < ENEMY_COLS; ci++) {
        uint8_t c = (start + ci) % ENEMY_COLS;
        /* Tìm con dưới cùng còn sống ở cột đó */
        for (int8_t r = ENEMY_ROWS-1; r >= 0; r--) {
            if (!en[r][c]) continue;
            /* Tìm slot đạn trống */
            for (uint8_t i = 0; i < MAX_E_BULLETS; i++) {
                if (!eb[i].on) {
                    eb[i].x  = ENEMY_START_X + e_ox + (int16_t)c*ENEMY_X_STEP + ENEMY_W/2;
                    eb[i].y  = ENEMY_START_Y + e_oy + (int16_t)r*ENEMY_Y_STEP + ENEMY_H;
                    eb[i].on = true;
                    /* Trộn seed bằng vị trí để tăng ngẫu nhiên */
                    rng_seed ^= (uint16_t)(eb[i].x + eb[i].y);
                    return;
                }
            }
            return;
        }
    }
}

static void boss_shoot() {
    uint8_t active = 0;
    for (uint8_t i = 0; i < MAX_E_BULLETS; i++) if (eb[i].on) active++;
    if (active >= MAX_E_BULLETS) return; 

    /* Randomize state periodically */
    if (rng8() % 10 < 3) b_state = rng8() % 3;

    if (b_state == 0) { /* Normal */
        for (uint8_t i = 0; i < MAX_E_BULLETS; i++) {
            if (!eb[i].on) {
                eb[i].x = b_x + rng8() % BOSS_W;
                eb[i].y = b_y + BOSS_H;
                eb[i].on = true;
                return;
            }
        }
    } else if (b_state == 1) { /* Aim at player */
        for (uint8_t i = 0; i < MAX_E_BULLETS; i++) {
            if (!eb[i].on) {
                eb[i].x = px; /* AIMED */
                eb[i].y = b_y + BOSS_H;
                eb[i].on = true;
                return;
            }
        }
    } else { /* Spread: 2 bullets */
        uint8_t fired = 0;
        for (uint8_t i = 0; i < MAX_E_BULLETS && fired < 2; i++) {
            if (!eb[i].on) {
                eb[i].x = (fired == 0) ? b_x : b_x + BOSS_W;
                eb[i].y = b_y + BOSS_H;
                eb[i].on = true;
                fired++;
            }
        }
    }
}

static bool hit_enemy(int16_t bx, int16_t by) {
    if (is_boss_level) {
        if (bx >= b_x && bx <= b_x + BOSS_W &&
            by <= b_y + BOSS_H &&
            (by + P_BULLET_SPD + 2) >= b_y) {
            
            b_hp--;
            g_score += SCORE_KILL;
            if (b_hp <= 0) {
                e_alive = 0;
            }
            BUZZER_PlaySound(BUZZER_SOUND_PEW);
            return true;
        }
        return false;
    }

    /* Duyệt từ hàng dưới cùng lên trên để viên đạn trúng con gần nhất trước */
    for (int8_t r = ENEMY_ROWS - 1; r >= 0; r--) {
        for (uint8_t c = 0; c < ENEMY_COLS; c++) {
            if (!en[r][c]) continue;
            int16_t ex = ENEMY_START_X + e_ox + (int16_t)c*ENEMY_X_STEP;
            int16_t ey = ENEMY_START_Y + e_oy + (int16_t)r*ENEMY_Y_STEP;
            
            /* 
             * Kiểm tra va chạm (tránh đạn xuyên qua do tốc độ quá nhanh).
             * Tọa độ Y của đạn trong frame này trải dài từ 'by' đến 'by + P_BULLET_SPD + 2'
             */
            if (bx >= ex && bx <= ex + ENEMY_W && 
                by <= ey + ENEMY_H && 
                (by + P_BULLET_SPD + 2) >= ey) {
                
                en[r][c] = false;
                e_alive--;
                g_score += SCORE_KILL;
                BUZZER_PlaySound(BUZZER_SOUND_PEW);
                return true;
            }
        }
    }
    return false;
}

static void game_update() {
    if (g_state == ST_LEVEL_CLEAR) {
        if (clr_ticks > 0) { clr_ticks--; }
        else { level_init(g_level + 1); }
        return;
    }
    if (g_state != ST_PLAYING) return;

    /* ---- Auto-repeat held keys ---- */
    if (g_keys_held & (KEY_LEFT | KEY_RIGHT)) {
        if (++g_key_move_tick >= KEY_REPEAT_MOVE_TICKS) {
            g_key_move_tick = 0;
            if (g_keys_held & KEY_LEFT) {
                px -= PLAYER_SPEED;
                if (px < PLAYER_X_MIN) px = PLAYER_X_MIN;
            }
            if (g_keys_held & KEY_RIGHT) {
                px += PLAYER_SPEED;
                if (px > PLAYER_X_MAX) px = PLAYER_X_MAX;
            }
        }
    } else {
        g_key_move_tick = 0;
    }

    if (g_keys_held & KEY_SHOOT) {
        if (++g_key_shoot_tick >= KEY_REPEAT_SHOOT_TICKS) {
            g_key_shoot_tick = 0;
            do_shoot();
        }
    } else {
        g_key_shoot_tick = 0;
    }

    /* player bullets */
    for (uint8_t i=0; i<MAX_P_BULLETS; i++) {
        if (!pb[i].on) continue;
        pb[i].y -= P_BULLET_SPD;
        if (pb[i].y < PLAY_TOP) { pb[i].on = false; continue; }
        if (hit_enemy(pb[i].x, pb[i].y)) { pb[i].on = false; }
    }

    /* enemy bullets */
    for (uint8_t i=0; i<MAX_E_BULLETS; i++) {
        if (!eb[i].on) continue;
        eb[i].y += e_bullet_spd;
        if (eb[i].y > GROUND_Y) { eb[i].on = false; continue; }
        if (eb[i].x >= px-3 && eb[i].x <= px+3 && eb[i].y >= GROUND_Y-4) {
            if (g_lives > 1) {
                g_lives--;
                BUZZER_PlaySound(BUZZER_SOUND_BANG);
                for (uint8_t j=0; j<MAX_E_BULLETS; j++) eb[j].on = false;
                break;
            } else {
                g_state = ST_GAME_OVER;
                BUZZER_StopLoop();
                BUZZER_PlaySound(BUZZER_SOUND_GAMEOVER);
                return;
            }
        }
    }

    /* move enemies / boss */
    if (is_boss_level) {
        if (++b_tick >= b_move_ticks) {
            b_tick = 0;
            e_anim = !e_anim;
            
            if (rng8() % 20 == 0) b_dir = -b_dir; /* Randomly change direction */
            
            b_x += ENEMY_MOVE_PX * b_dir;
            if (b_x <= 0) { b_x = 0; b_dir = 1; }
            if (b_x + BOSS_W >= LCD_WIDTH - 1) { b_x = LCD_WIDTH - 1 - BOSS_W; b_dir = -1; }
        }
    } else {
        if (++e_tick >= e_move_ticks) {
            e_tick = 0;
            e_anim = !e_anim;
            int16_t left, right, bot;
            enemy_bounds(left, right, bot);
            bool wall = (e_dir>0 && right+ENEMY_MOVE_PX >= LCD_WIDTH-1)
                     || (e_dir<0 && left-ENEMY_MOVE_PX <= 0);
            if (wall) {
                e_oy += ENEMY_DROP_PX;
                e_dir = -e_dir;
                if (bot + ENEMY_DROP_PX >= GROUND_Y - 6) {
                    g_state = ST_GAME_OVER;
                    BUZZER_StopLoop();
                    BUZZER_PlaySound(BUZZER_SOUND_GAMEOVER);
                    return;
                }
            } else {
                e_ox += ENEMY_MOVE_PX * e_dir;
            }
        }
    }

    /* enemy/boss shooting */
    if (is_boss_level) {
        if (++b_shoot_t >= b_shoot_iv) {
            b_shoot_t = 0;
            boss_shoot();
        }
    } else {
        if (++e_shoot_t >= e_shoot_iv) {
            e_shoot_t = 0;
            enemy_shoot_one();
        }
    }

    /* level clear? */
    if (e_alive == 0) {
        g_score  += SCORE_LEVEL;
        g_state   = ST_LEVEL_CLEAR;
        clr_ticks = 20; /* 20 * 80ms = 1.6s */
        BUZZER_PlaySound(BUZZER_SOUND_WELCOME);
    }
}

static void do_shoot() {
    for (uint8_t i=0; i<MAX_P_BULLETS; i++) {
        if (!pb[i].on) {
            if (g_level >= 7) {
                pb[i].x = p_alt_gun ? px - 3 : px + 3;
                p_alt_gun = !p_alt_gun;
            } else {
                pb[i].x = px;
            }
            pb[i].y = GROUND_Y-8; pb[i].on = true;
            return;
        }
    }
}

/* ========== SCREEN HANDLER ========== */

void scr_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY:
        game_reset();
        g_keys_held = 0;
        g_key_move_tick = 0;
        g_key_shoot_tick = 0;
        timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UPDATE,
                  AC_DISPLAY_GAME_UPDATE_INTERVAL, TIMER_PERIODIC);
        break;

    case AC_DISPLAY_GAME_UPDATE:
        game_update();
        break;

    /* ---- PRESSED: hành động ngay + set cờ held ---- */
    case AC_DISPLAY_BUTON_UP_PRESSED:
        if (g_state == ST_GAME_OVER) { game_reset(); break; }
        g_keys_held |= KEY_LEFT;
        g_key_move_tick = KEY_REPEAT_MOVE_TICKS; /* fire ngay tick tiếp theo */
        px -= PLAYER_SPEED;
        if (px < PLAYER_X_MIN) px = PLAYER_X_MIN;
        break;

    case AC_DISPLAY_BUTON_DOWN_PRESSED:
        if (g_state == ST_GAME_OVER) { game_reset(); break; }
        g_keys_held |= KEY_RIGHT;
        g_key_move_tick = KEY_REPEAT_MOVE_TICKS;
        px += PLAYER_SPEED;
        if (px > PLAYER_X_MAX) px = PLAYER_X_MAX;
        break;

    case AC_DISPLAY_BUTON_MODE_PRESSED:
        if (g_state == ST_GAME_OVER) { game_reset(); break; }
        if (g_state == ST_PLAYING) {
            g_keys_held |= KEY_SHOOT;
            g_key_shoot_tick = KEY_REPEAT_SHOOT_TICKS; /* fire ngay */
            do_shoot();
        }
        break;

    /* ---- RELEASED: xóa cờ held ---- */
    case AC_DISPLAY_BUTON_UP_RELEASED:
        g_keys_held &= (uint8_t)~KEY_LEFT;
        break;

    case AC_DISPLAY_BUTON_DOWN_RELEASED:
        g_keys_held &= (uint8_t)~KEY_RIGHT;
        break;

    case AC_DISPLAY_BUTON_MODE_RELEASED:
        g_keys_held &= (uint8_t)~KEY_SHOOT;
        break;

    default: break;
    }
}
