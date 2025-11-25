// menu.c — minimal version
#include "menu.h"
#include <stdio.h>
#include "lcd.h"
#include "com.h"

#ifndef WHITE
#define WHITE 0xFFFF
#endif

// -------- input bitmask (these bits must match what main passes to menu_update) -----
#define BTN_LEFT    (1u << 0)
#define BTN_RIGHT   (1u << 1)
#define BTN_SELECT  (1u << 4)
#define BTN_BACK    (1u << 5)

// -------- tiny 2-byte protocol: {type, color} --------------------------------------
typedef enum {
    MENU_MSG_READY  = 1,
    MENU_MSG_CANCEL = 2
} menu_msg_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;   // menu_msg_type_t
    uint8_t color;  // color_selection_t
} menu_msg_t;

// local shadow (not exposed in menu.h)
static bool s_local_ready = false;
static bool s_peer_ready  = false;
static color_selection_t s_peer_color = -1;  // -1 = unknown
static inline const char* color_name(color_selection_t c) {
    return (c == COLOR_BLUE) ? "BLUE" : "RED";
}

static void menu_send(uint8_t type, color_selection_t color) {
    menu_msg_t m = { type, (uint8_t)color };
    (void)com_write(&m, sizeof(m)); // assume non-blocking or short
}

static void menu_poll_peer(menu_t *menu) {
    menu_msg_t m;
    int32_t n = com_read(&m, sizeof(m));
    if (n != (int32_t)sizeof(m)) return; // ignore nothing/short reads

    if (m.type == MENU_MSG_READY) {
        s_peer_ready = true;
        s_peer_color = (color_selection_t)m.color;  // lock their color
    }
    if (m.type == MENU_MSG_CANCEL) {
        s_peer_ready = false;
        s_peer_color = -1;  // unlock
    }

    menu->both_ready = (s_local_ready && s_peer_ready);
    if (menu->both_ready) menu->state = MENU_STATE_READY;
}

// ------------------- public API -------------------
void menu_init(menu_t *menu) {
    if (!menu) return;
    menu->state      = MENU_STATE_COLOR_SELECT;
    menu->color      = COLOR_BLUE;
    menu->both_ready = false;
    s_local_ready    = false;
    s_peer_ready     = false;
    s_peer_color     = -1;
}

void menu_set_color(menu_t *menu, color_selection_t color) {
    if (!menu) return;
    menu->color = color;
}

void menu_update(menu_t *menu, uint8_t input) {
    if (!menu) return;

    // check peer first (no-blocking)
    menu_poll_peer(menu);

    switch (menu->state) {
    case MENU_STATE_COLOR_SELECT:
        if (input & (BTN_LEFT | BTN_RIGHT)) {
            menu->color = (menu->color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE;
            // skip if peer already took this color
            if (s_peer_ready && menu->color == s_peer_color) {
                menu->color = (menu->color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE;
            }
        }
        if (input & BTN_SELECT) {
            s_local_ready    = true;
            menu->both_ready = (s_local_ready && s_peer_ready);
            menu->state      = menu->both_ready ? MENU_STATE_READY : MENU_STATE_WAITING;
            menu_send(MENU_MSG_READY, menu->color);
        }
        break;

    case MENU_STATE_WAITING:
        if (input & BTN_BACK) {
            s_local_ready    = false;
            menu->both_ready = false;
            menu->state      = MENU_STATE_COLOR_SELECT;
            menu_send(MENU_MSG_CANCEL, menu->color);
        }
        // peer readiness flips state inside menu_poll_peer()
        break;

    case MENU_STATE_READY:
        // optional: allow un-ready before game start
        if (input & BTN_BACK) {
            s_local_ready    = false;
            s_peer_ready     = false;      // optional: force peer re-handshake
            menu->both_ready = false;
            menu->state      = MENU_STATE_COLOR_SELECT;
            menu_send(MENU_MSG_CANCEL, menu->color);
        }
        break;
    }
}

void menu_draw(menu_t *menu) {
    if (!menu) return;
    int y = 0;

    switch (menu->state) {
    case MENU_STATE_COLOR_SELECT: {
        lcd_drawString(0, y, "SNAKE - MULTIPLAYER", WHITE); y += 12;
        lcd_drawString(0, y, "Select Color:", WHITE);       y += 12;
        char line[24];
        snprintf(line, sizeof(line), "> %s <", color_name(menu->color));
        lcd_drawString(0, y, line, WHITE); y += 12;
        if (s_peer_ready) {
            char taken[24];
            snprintf(taken, sizeof(taken), "Peer took: %s", color_name(s_peer_color));
            lcd_drawString(0, y, taken, WHITE); y += 12;
        }
        lcd_drawString(0, y, "Left/Right: Toggle", WHITE); y += 12;
        lcd_drawString(0, y, "A: Ready", WHITE);
        break;
    }
    case MENU_STATE_WAITING:
        lcd_drawString(0, y, "Waiting for peer...", WHITE); y += 12;
        lcd_drawString(0, y, s_local_ready ? "You: READY" : "You: —", WHITE); y += 12;
        lcd_drawString(0, y, s_peer_ready  ? "Peer: READY" : "Peer: —", WHITE);
        break;

    case MENU_STATE_READY: {
        lcd_drawString(0, y, "Both ready!", WHITE); y += 12;
        char vs[32];
        snprintf(vs, sizeof(vs), "%s vs %s",
                 color_name(menu->color),
                 s_peer_ready ? (menu->color == COLOR_BLUE ? "RED" : "BLUE") : "?");
        lcd_drawString(0, y, vs, WHITE);
        break;
    }
    }
}

bool menu_is_ready(menu_t *menu) {
    if (!menu) return false;
    menu->both_ready = (s_local_ready && s_peer_ready);
    return menu->both_ready;
}
