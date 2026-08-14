/*
 * scr_start.cpp — Game Start Screen
 * Hiển thị màn hình bắt đầu game với alien sprites và prompt nhấn MODE.
 * Layout (128x64 OLED):
 *   y= 0-11 : Title box "** ALIEN SHOOTER **"
 *   y=14-26 : 2 hàng alien sprites
 *   y=29    : separator line
 *   y=37-55 : Blinking "PRESS [MODE]" / "TO  START"
 */
#include "scr_start.h"

/* ---- Kích thước màn hình ---- */
#define SCR_W   128
#define SCR_H   64

/* ---- Alien formation ---- */
#define S_COLS       6
#define S_X_STEP    16          /* bước ngang giữa hai alien */
#define S_START_X   20          /* x đầu tiên: (128 - 5*16 - 7) / 2 = 20 */
#define S_ROW0_Y    15          /* hàng alien trên */
#define S_ROW1_Y    22          /* hàng alien dưới */

/* ---- State ---- */
static bool s_blink = false;    /* toggle prompt text */
static bool s_anim  = false;    /* toggle alien anim frame */

static void view_scr_start();

view_dynamic_t dyn_view_start = {{ .item_type = ITEM_TYPE_DYNAMIC }, view_scr_start};
view_screen_t  scr_start      = {&dyn_view_start, ITEM_NULL, ITEM_NULL, .focus_item = 0};

/* ---- Helper: vẽ một alien (copy từ scr_game, dùng lại không include) ---- */
static void draw_alien(int16_t x, int16_t y, uint8_t row, bool anim) {
    if (row == 0) {
        /* kiểu ^^ (hàng trên) */
        view_render.drawPixel(x+1, y + (anim ? 0 : 1), WHITE);
        view_render.drawPixel(x+5, y + (anim ? 0 : 1), WHITE);
        view_render.drawLine(x, y+1, x+6, y+1, WHITE);
        view_render.drawPixel(x+2, y+2, WHITE);
        view_render.drawPixel(x+4, y+2, WHITE);
    } else {
        /* kiểu rect (hàng dưới) */
        view_render.drawRect(x+1, y, 5, 3, WHITE);
        view_render.drawPixel(anim ? x : x+6, y+1, WHITE);
        view_render.drawPixel(anim ? x+6 : x, y+1, WHITE);
    }
}

/* ---- Vẽ màn hình ---- */
static void view_scr_start() {
    view_render.clear();
    view_render.setTextColor(WHITE);
    view_render.setTextSize(1);

    /* === Khung tiêu đề === */
    view_render.drawRect(0, 0, SCR_W, 12, WHITE);
    /* "** ALIEN SHOOTER **" = 19 chars × 6px = 114px  →  x=(128-114)/2 = 7 */
    view_render.setCursor(7, 2);
    view_render.print("** ALIEN SHOOTER **");

    /* === Đội hình alien 2 hàng === */
    for (uint8_t c = 0; c < S_COLS; c++) {
        int16_t ex = S_START_X + (int16_t)c * S_X_STEP;
        draw_alien(ex, S_ROW0_Y, 0, s_anim);
        draw_alien(ex, S_ROW1_Y, 1, s_anim);
    }

    /* === Đường kẻ phân cách === */
    view_render.drawLine(0, 29, SCR_W - 1, 29, WHITE);

    /* === Prompt nhấn nút (blink) === */
    if (s_blink) {
        /* "PRESS [MODE]" = 12 chars  →  x=(128-72)/2 = 28 */
        view_render.setCursor(28, 37);
        view_render.print("PRESS [MODE]");

        /* "TO  START" = 9 chars  →  x=(128-54)/2 = 37 */
        view_render.setCursor(37, 49);
        view_render.print("TO  START");
    } else {
        /* Luôn hiện dòng gợi ý nhỏ khi tắt để user biết nút nào */
        view_render.setCursor(28, 37);
        view_render.print("PRESS [MODE]");
        view_render.setCursor(37, 49);
        view_render.print("TO  START");
        /* Vẽ underline nhấp nháy thay vì ẩn chữ */
        view_render.drawLine(28, 46, 99, 46, WHITE);
    }
}

/* ---- Handler ---- */
void scr_start_handle(ak_msg_t* msg) {
    switch (msg->sig) {

    case SCREEN_ENTRY: {
        APP_DBG_SIG("scr_start SCREEN_ENTRY\n");
        s_blink = true;
        s_anim  = false;
        /* Timer nhấp nháy 500ms */
        timer_set(AC_TASK_DISPLAY_ID,
                  AC_DISPLAY_START_BLINK_TICK,
                  AC_DISPLAY_START_BLINK_INTERVAL,
                  TIMER_PERIODIC);
    } break;

    case AC_DISPLAY_START_BLINK_TICK: {
        s_blink = !s_blink;
        s_anim  = !s_anim;
    } break;

    /* Nhấn MODE → bắt đầu game */
    case AC_DISPLAY_BUTON_MODE_PRESSED: {
        APP_DBG_SIG("scr_start → scr_game\n");
        timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_START_BLINK_TICK);
        BUZZER_PlaySound(BUZZER_SOUND_LETS_GO);
        SCREEN_TRAN(scr_game_handle, &scr_game);
    } break;

    /* UP/DOWN không làm gì trên màn này */
    case AC_DISPLAY_BUTON_UP_PRESSED:
    case AC_DISPLAY_BUTON_DOWN_PRESSED:
    default:
        break;
    }
}
