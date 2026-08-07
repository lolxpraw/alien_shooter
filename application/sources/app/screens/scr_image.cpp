/*
 * scr_image.cpp
 *
 * Screen hiển thị hình ảnh STM32 chip đơn giản trên OLED 128x64.
 * Bitmap vẽ hình chip IC với các chân pin xung quanh.
 */

#include "scr_image.h"

static void view_scr_image();

view_dynamic_t dyn_view_image = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_image
};

view_screen_t scr_image = {
	&dyn_view_image,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

/*
 * Vẽ hình chip STM32 bằng các primitive draw của view_render.
 * Màn hình OLED: 128 x 64 pixels.
 *
 * Layout:
 *   - Chip body: hình chữ nhật 48x40 px, căn giữa màn hình → x=40, y=12
 *   - Chân pin: mỗi bên có 6 chân, cách đều nhau
 *   - Nhãn "STM32" in bên trong chip
 */
void view_scr_image() {
#define CHIP_X      (38)     /* tọa độ x góc trái trên của chip body */
#define CHIP_Y      (10)     /* tọa độ y góc trái trên của chip body */
#define CHIP_W      (48)     /* chiều rộng chip body */
#define CHIP_H      (40)     /* chiều cao chip body */
#define PIN_LEN     (6)      /* chiều dài mỗi chân pin */
#define PIN_SPACING (6)      /* khoảng cách giữa các chân */
#define PIN_COUNT   (6)      /* số chân mỗi bên */

	view_render.clear();

	/* ----- Vẽ thân chip (2 lớp viền để trông dày hơn) ----- */
	view_render.drawRect(CHIP_X, CHIP_Y, CHIP_W, CHIP_H, WHITE);
	view_render.drawRect(CHIP_X + 1, CHIP_Y + 1, CHIP_W - 2, CHIP_H - 2, WHITE);

	/* ----- Chấm chỉ vị trí pin 1 (góc trái trên) ----- */
	view_render.drawCircle(CHIP_X + 4, CHIP_Y + 4, 2, WHITE);

	/* ----- Chân pin bên TRÁI (6 chân) ----- */
	for (uint8_t i = 0; i < PIN_COUNT; i++) {
		int16_t pin_y = CHIP_Y + 4 + i * PIN_SPACING;
		/* nét ngang của chân */
		view_render.drawLine(CHIP_X - PIN_LEN, pin_y, CHIP_X, pin_y, WHITE);
		/* đầu chân (vuông nhỏ) */
		view_render.drawPixel(CHIP_X - PIN_LEN - 1, pin_y, WHITE);
	}

	/* ----- Chân pin bên PHẢI (6 chân) ----- */
	for (uint8_t i = 0; i < PIN_COUNT; i++) {
		int16_t pin_y = CHIP_Y + 4 + i * PIN_SPACING;
		view_render.drawLine(CHIP_X + CHIP_W, pin_y, CHIP_X + CHIP_W + PIN_LEN, pin_y, WHITE);
		view_render.drawPixel(CHIP_X + CHIP_W + PIN_LEN + 1, pin_y, WHITE);
	}

	/* ----- Chân pin bên TRÊN (5 chân) ----- */
	for (uint8_t i = 0; i < 5; i++) {
		int16_t pin_x = CHIP_X + 6 + i * PIN_SPACING;
		view_render.drawLine(pin_x, CHIP_Y - PIN_LEN, pin_x, CHIP_Y, WHITE);
		view_render.drawPixel(pin_x, CHIP_Y - PIN_LEN - 1, WHITE);
	}

	/* ----- Chân pin bên DƯỚI (5 chân) ----- */
	for (uint8_t i = 0; i < 5; i++) {
		int16_t pin_x = CHIP_X + 6 + i * PIN_SPACING;
		view_render.drawLine(pin_x, CHIP_Y + CHIP_H, pin_x, CHIP_Y + CHIP_H + PIN_LEN, WHITE);
		view_render.drawPixel(pin_x, CHIP_Y + CHIP_H + PIN_LEN + 1, WHITE);
	}

	/* ----- Nhãn "STM32" bên trong chip ----- */
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	view_render.setCursor(CHIP_X + 6, CHIP_Y + 13);
	view_render.print("STM32");

	/* ----- Dòng chữ nhỏ phía dưới nhãn ----- */
	view_render.setCursor(CHIP_X + 3, CHIP_Y + 24);
	view_render.print("F103C8");
}

void scr_image_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		/* Màn hình tĩnh, không cần timer */
	} break;

	case AC_DISPLAY_BUTON_MODE_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_PRESSED\n");
		SCREEN_BACK();
	} break;

	case AC_DISPLAY_BUTON_UP_PRESSED:
	case AC_DISPLAY_BUTON_DOWN_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTON_PRESSED\n");
		SCREEN_BACK();
	} break;

	default:
		break;
	}
}
