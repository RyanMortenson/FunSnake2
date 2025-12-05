// menu.c — minimal version
#include "menu.h"
#include <stdio.h>
#include "lcd.h"
#include "com.h"
#include "config.h"

#ifndef WHITE
#define WHITE 0xFFFF
#endif
#ifndef BLUE
#define BLUE rgb565(0, 0, 31)
#endif
#ifndef RED
#define RED rgb565(31, 0, 0)
#endif
#ifndef GREEN
#define GREEN rgb565(0, 31, 0)
#endif
#ifndef CYAN
#define CYAN rgb565(0, 31, 31)
#endif
#ifndef YELLOW
#define YELLOW rgb565(31, 31, 0)
#endif
#ifndef LIGHTGREY
#define LIGHTGREY rgb565(20, 20, 20)
#endif

// -------- input bitmask (these bits must match what main passes to menu_update) -----
#define BTN_LEFT    (1u << 0)
#define BTN_RIGHT   (1u << 1)
#define BTN_SELECT  (1u << 4)
#define BTN_BACK    (1u << 5)

// -------- tiny 2-byte protocol: {type, color} --------------------------------------
// typedef enum {
//     MENU_MSG_READY  = 1,
//     MENU_MSG_CANCEL = 2
// } menu_msg_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;   // menu_msg_type_t
    uint8_t color;  // color_selection_t
} menu_msg_t;

// local shadow (not exposed in menu.h)
static bool s_local_ready = false;
static bool s_peer_ready  = false;
static bool s_local_start = false;
static bool s_peer_start  = false;
static color_selection_t s_peer_color = -1;  // -1 = unknown
static uint8_t s_prev_input = 0;             // edge detection
static bool s_dirty = true;                  // redraw flag
static inline const char* color_name(color_selection_t c) {
    return (c == COLOR_BLUE) ? "BLUE" : "RED";
}

static void draw_panel(coord_t x, coord_t y, coord_t w, coord_t h, color_t base, const char *label, bool selected, bool disabled) {
    color_t border = disabled ? rgb565(32, 0, 0) : base;
    color_t fill   = disabled ? rgb565(12, 12, 12) : base;
    lcd_fillRoundRect(x, y, w, h, 6, fill);
    lcd_drawRoundRect(x, y, w, h, 6, border);
    if (selected) {
        lcd_drawRoundRect(x-2, y-2, w+4, h+4, 8, YELLOW);
    }
    coord_t tx = x + 8;
    coord_t ty = y + (h/2 - LCD_CHAR_H);
    lcd_drawString(tx, ty, label, WHITE);
    if (disabled) {
        lcd_drawString(tx, ty + LCD_CHAR_H + 2, "(TAKEN)", RED);
    }
}

static void menu_send(uint8_t type, color_selection_t color) {
    menu_msg_t m = { type, (uint8_t)color };
    (void)com_write(&m, sizeof(m)); // assume non-blocking or short
}

static bool menu_poll_peer(menu_t *menu) {
    menu_msg_t m;
    int32_t n = com_read(&m, sizeof(m));
    if (n != (int32_t)sizeof(m)) return false; // ignore nothing/short reads

    bool prev_peer_ready  = s_peer_ready;
    color_selection_t prev_peer_color = s_peer_color;
    bool prev_peer_start = s_peer_start;
    menu_state_t prev_state = menu->state;
    bool prev_both_ready = menu->both_ready;

    if (m.type == MENU_MSG_READY) {
        s_peer_ready = true;
        s_peer_color = (color_selection_t)m.color;  // lock their color
        s_peer_start = false;  // reset start confirmation when re-ready
    }
    if (m.type == MENU_MSG_CANCEL) {
        s_peer_ready = false;
        s_peer_start = false;
        s_peer_color = -1;  // unlock
    }
    if (m.type == MENU_MSG_START) {
        s_peer_start = true;
    }
    if (m.type == MENU_MSG_START_CANCEL) {
        s_peer_start = false;
    }

    bool color_ready = (s_local_ready && s_peer_ready);
    menu->both_ready = (color_ready && s_local_start && s_peer_start);
    if (color_ready) {
        menu->state = MENU_STATE_READY;
    } else {
        menu->state = s_local_ready ? MENU_STATE_WAITING : MENU_STATE_COLOR_SELECT;
        s_local_start = false;
        s_peer_start = false;
    }

    if (prev_peer_ready != s_peer_ready ||
        prev_peer_color != s_peer_color ||
        prev_peer_start != s_peer_start ||
        prev_state != menu->state ||
        prev_both_ready != menu->both_ready) {
        s_dirty = true;
        return true;
    }

    return false;
}

// ------------------- public API -------------------
void menu_init(menu_t *menu) {
    if (!menu) return;
    menu->state      = MENU_STATE_COLOR_SELECT;
    menu->color      = COLOR_BLUE;
    menu->both_ready = false;
    s_local_ready    = false;
    s_peer_ready     = false;
    s_local_start    = false;
    s_peer_start     = false;
    s_peer_color     = -1;
    s_prev_input     = 0;
    s_dirty          = true;
}

void menu_set_color(menu_t *menu, color_selection_t color) {
    if (!menu) return;
    menu->color = color;
}

void menu_update(menu_t *menu, uint8_t input) {
    if (!menu) return;

    uint8_t pressed = input & ~s_prev_input; // only act on rising edges
    s_prev_input = input;

    // check peer first (no-blocking)
    menu_poll_peer(menu);

    bool changed = false;

    switch (menu->state) {
    case MENU_STATE_COLOR_SELECT:
        if (pressed & (BTN_LEFT | BTN_RIGHT)) {
            menu->color = (menu->color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE;
            // skip if peer already took this color
            if (s_peer_ready && menu->color == s_peer_color) {
                menu->color = (menu->color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE;
            }
            changed = true;
        }
        if (pressed & BTN_SELECT) {
            s_local_ready    = true;
            s_local_start    = false;
            menu->both_ready = (s_local_ready && s_peer_ready);
            menu->state      = menu->both_ready ? MENU_STATE_READY : MENU_STATE_WAITING;
            menu_send(MENU_MSG_READY, menu->color);
            changed = true;
        }
        break;

    case MENU_STATE_WAITING:
        if (pressed & BTN_BACK) {
            s_local_ready    = false;
            s_local_start    = false;
            menu->both_ready = false;
            menu->state      = MENU_STATE_COLOR_SELECT;
            menu_send(MENU_MSG_CANCEL, menu->color);
            changed = true;
        }
        // peer readiness flips state inside menu_poll_peer()
        break;

    case MENU_STATE_READY:
        // optional: allow un-ready before game start
        if (pressed & BTN_BACK) {
            s_local_ready    = false;
            s_peer_ready     = false;      // optional: force peer re-handshake
            s_local_start    = false;
            s_peer_start     = false;
            menu->both_ready = false;
            menu->state      = MENU_STATE_COLOR_SELECT;
            menu_send(MENU_MSG_CANCEL, menu->color);
            changed = true;
        } else if (pressed & BTN_SELECT) {
            // re-open color selection if both already ready but want to change
            s_local_ready = false;
            s_local_start = false;
            menu->state   = MENU_STATE_COLOR_SELECT;
            menu_send(MENU_MSG_CANCEL, menu->color);
            changed = true;
        } else if (pressed & BTN_RIGHT) { // START button
            s_local_start = true;
            menu_send(MENU_MSG_START, menu->color);
            menu->both_ready = (s_local_ready && s_peer_ready && s_local_start && s_peer_start);
            changed = true;
        }
        break;
    }

    if (changed) {
        s_dirty = true;
    }
}

void menu_draw(menu_t *menu) {
    if (!menu) return;
    if (!s_dirty) return;
    lcd_fillScreen(CONFIG_COLOR_BACKGROUND);
    lcd_setFontSize(2);

    // Header
    int y = 6;
    lcd_drawString(8, y, "SNAKE - MULTIPLAYER", CYAN); y += 20;
    lcd_setFontSize(1);

    // Color cards
    const coord_t card_w = 90;
    const coord_t card_h = 60;
    const coord_t top    = y;
    draw_panel(12, top, card_w, card_h, BLUE, "BLUE", menu->color == COLOR_BLUE, s_peer_ready && s_peer_color == COLOR_BLUE);
    draw_panel(LCD_W - card_w - 12, top, card_w, card_h, RED, "RED", menu->color == COLOR_RED, s_peer_ready && s_peer_color == COLOR_RED);

    // Status
    y = top + card_h + 10;
    lcd_drawString(8, y, "You:", WHITE);
    lcd_drawString(48, y, s_local_ready ? "READY" : "Choosing", s_local_ready ? GREEN : WHITE); y += 12;
    lcd_drawString(8, y, "Peer:", WHITE);
    lcd_drawString(48, y, s_peer_ready ? "READY" : "Waiting", s_peer_ready ? GREEN : WHITE); y += 14;
    lcd_drawString(8, y, "Start:", WHITE);
    lcd_drawString(48, y, s_local_start ? "Pressed" : "Waiting", s_local_start ? GREEN : WHITE); y += 12;
    lcd_drawString(8, y, "Peer Start:", WHITE);
    lcd_drawString(88, y, s_peer_start ? "Pressed" : "Waiting", s_peer_start ? GREEN : WHITE); y += 14;

    // Bottom instructions/state
    switch (menu->state) {
    case MENU_STATE_COLOR_SELECT:
        lcd_drawString(8, y, "OPTION: Left   START: Right", LIGHTGREY); y += 12;
        lcd_drawString(8, y, "A: Ready   B: Back", LIGHTGREY);
        break;
    case MENU_STATE_WAITING:
        lcd_drawString(8, y, "Waiting for peer...", YELLOW); y += 12;
        lcd_drawString(8, y, "B: Change color", LIGHTGREY);
        break;
    case MENU_STATE_READY: {
        char vs[32];
        snprintf(vs, sizeof(vs), "%s vs %s",
                 color_name(menu->color),
                 s_peer_ready ? (menu->color == COLOR_BLUE ? "RED" : "BLUE") : "?");
        lcd_drawString(8, y, "Both ready!", GREEN); y += 12;
        lcd_drawString(8, y, vs, WHITE); y += 12;
        if (menu->both_ready) {
            lcd_drawString(8, y, "Starting game...", GREEN);
        } else {
            lcd_drawString(8, y, "Press START on both devices", LIGHTGREY);
        }
        break;
    }
    }

    s_dirty = false;
}

bool menu_is_ready(menu_t *menu) {
    if (!menu) return false;
    menu->both_ready = (s_local_ready && s_peer_ready && s_local_start && s_peer_start);
    return menu->both_ready;
}
