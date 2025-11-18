// app_main.c — minimal menu runner
#include "menu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcd.h"   // must provide: lcd_clear(), lcd_drawString(int x,int y,const char*, uint16_t)

#define WHITE 0xFFFF

// Map these bits to your board's buttons in buttons_read()
#define BTN_LEFT    (1u << 0)
#define BTN_RIGHT   (1u << 1)
#define BTN_SELECT  (1u << 4)
// Optional: press this to fake the peer being ready (no Wi-Fi needed)
#define BTN_FAKE_PEER_READY (1u << 6)

extern uint8_t buttons_read(void);  // you implement this for your board

static void draw_menu(const menu_t *m) {
  lcd_clear();
  int y = 0;

  switch (m->state) {
  case MENU_STATE_COLOR_SELECT:
    lcd_drawString(0,y, "Select Color", WHITE); y += 12;
    lcd_drawString(0,y, (m->color == COLOR_BLUE) ? "BLUE" : "RED", WHITE); y += 12;
    lcd_drawString(0,y, "LEFT/RIGHT: toggle", WHITE); y += 12;
    lcd_drawString(0,y, "A/SELECT: ready", WHITE);
    break;

  case MENU_STATE_WAITING:
    lcd_drawString(0,y, "Waiting for peer...", WHITE); y += 12;
    lcd_drawString(0,y, m->both_ready ? "Both ready!" : "(press Y to fake peer)", WHITE);
    break;

  case MENU_STATE_READY:
    lcd_drawString(0,y, "Both READY!", WHITE); y += 12;
    lcd_drawString(0,y, "Hand control to game FSM", WHITE);
    break;
  }
}

void app_main(void) {
  // init your LCD here if needed (e.g., lcd_init())
  menu_t menu;
  menu_init(&menu);   // your menu.c sets default state/color

  for (;;) {
    uint8_t in = buttons_read();

    // Toggle color locally
    if (in & (BTN_LEFT | BTN_RIGHT)) {
      menu_set_color(&menu, (menu.color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE);
    }

    // Tell the menu we pressed SELECT (your menu_update handles state change)
    if (in & BTN_SELECT) {
      menu_update(&menu, BTN_SELECT);
    }

    // Optional: fake the peer being ready so you can see READY screen without Wi-Fi
    if (in & BTN_FAKE_PEER_READY) {
      menu.both_ready = true;
      menu.state = MENU_STATE_READY;
    }

    draw_menu(&menu);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
