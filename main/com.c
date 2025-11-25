// com.c — ultra-minimal color picker FSM
#include "menu.h"
#include "lcd.h"

#ifndef WHITE
#define WHITE 0xFFFF
#endif

// Map these bits to your board in main() and pass them to menu_update()
#define BTN_LEFT    (1u << 0)
#define BTN_RIGHT   (1u << 1)
#define BTN_SELECT  (1u << 4)
#define BTN_BACK    (1u << 5)

static inline const char* color_name(color_selection_t c) {
    return (c == COLOR_BLUE) ? "BLUE" : "RED";
}

void menu_init(menu_t *m) {
    if (!m) return;
    m->state      = MENU_STATE_COLOR_SELECT;  // only two states used
    m->color      = COLOR_BLUE;               // default
    m->both_ready = false;                    // “ready” = user confirmed
}

void menu_set_color(menu_t *m, color_selection_t c) {
    if (!m) return;
    m->color = c;
}

void menu_update(menu_t *m, uint8_t input) {
    if (!m) return;

    switch (m->state) {
    case MENU_STATE_COLOR_SELECT:
        // Toggle color with LEFT/RIGHT
        if (input & (BTN_LEFT | BTN_RIGHT)) {
            m->color = (m->color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE;
        }
        // SELECT -> ready
        if (input & BTN_SELECT) {
            m->both_ready = true;
            m->state = MENU_STATE_READY;
        }
        break;

    case MENU_STATE_READY:
        // Optional: BACK to un-ready and change color again
        if (input & BTN_BACK) {
            m->both_ready = false;
            m->state = MENU_STATE_COLOR_SELECT;
        }
        break;

    default:
        // Safety: snap to a known state
        m->state = MENU_STATE_COLOR_SELECT;
        break;
    }
}

void menu_draw(menu_t *m) {
    if (!m) return;

    int y = 0;
    switch (m->state) {
    case MENU_STATE_COLOR_SELECT:
        lcd_drawString(0, y, "Select Color", WHITE); y += 12;
        lcd_drawString(0, y, (m->color == COLOR_BLUE) ? "BLUE" : "RED", WHITE); y += 12;
        lcd_drawString(0, y, "LEFT/RIGHT: toggle", WHITE); y += 12;
        lcd_drawString(0, y, "SELECT: ready", WHITE);
        break;

    case MENU_STATE_READY:
        lcd_drawString(0, y, "Ready!", WHITE); y += 12;
        lcd_drawString(0, y, color_name(m->color), WHITE);
        break;
    }
}

bool menu_is_ready(menu_t *m) {
    return m && m->both_ready;
}

// Game protocol helpers
void com_send_move(uint8_t dir, uint8_t x, uint8_t y) {
    game_msg_t m = { GAME_MSG_MOVE, dir, x, y };
    com_write(&m, sizeof(m));
}

void com_send_fruit(uint8_t x, uint8_t y) {
    game_msg_t m = { GAME_MSG_FRUIT, 0, x, y };
    com_write(&m, sizeof(m));
}

// Returns NULL if no message, or pointer to game_msg_t if received
game_msg_t* com_recv_game(void) {
    static game_msg_t m;
    int32_t n = com_read(&m, sizeof(m));
    if (n != (int32_t)sizeof(m)) return NULL;
    return &m;
}
