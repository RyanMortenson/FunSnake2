// menu.c
#include "menu.h"

#include <string.h>
#include <stdio.h>

#include "lcd.h"   // assumes lcd_drawString(int x,int y,const char*, uint16_t color)
#include "com.h"   // assumes com_write(const void*, uint32_t), com_read(void*, uint32_t)
#include "esp_log.h"

#define TAG "menu"

// ---------- INPUT BITMASK (adjust to your buttons/joystick) ----------
#define BTN_LEFT    (1u << 0)
#define BTN_RIGHT   (1u << 1)
#define BTN_UP      (1u << 2)
#define BTN_DOWN    (1u << 3)
#define BTN_SELECT  (1u << 4)   // confirm / A
#define BTN_BACK    (1u << 5)   // cancel  / B

// ---------- SMALL PROTOCOL FOR MENU HANDSHAKE ----------
typedef enum : uint8_t {
    MENU_MSG_READY  = 1,
    MENU_MSG_CANCEL = 2,
    MENU_MSG_HELLO  = 3
} menu_msg_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;               // menu_msg_type_t
    uint8_t color;              // color_selection_t
    uint8_t reserved0;
    uint8_t reserved1;
} menu_msg_t;

// local shadow state not exposed in menu.h
static bool               s_local_ready = false;
static bool               s_peer_ready  = false;
static color_selection_t  s_peer_color  = COLOR_BLUE;

// ---------- helpers ----------
static inline const char* color_name(color_selection_t c) {
    switch (c) {
        case COLOR_BLUE: return "BLUE";
        case COLOR_RED:  return "RED";
        default:         return "?";
    }
}

static void menu_send(uint8_t type, color_selection_t color) {
    menu_msg_t m = { .type = type, .color = (uint8_t)color, .reserved0 = 0, .reserved1 = 0 };
    (void)com_write(&m, sizeof(m));  // fire-and-forget
}

static void menu_poll_peer(menu_t *menu) {
    // Non-blocking read; if your com_read blocks, add a non-blocking flag in com_read.
    menu_msg_t m;
    int32_t n = com_read(&m, sizeof(m));
    if (n <= 0) return; // nothing yet

    if ((uint32_t)n < sizeof(m)) {
        ESP_LOGW(TAG, "Short menu packet: %ld", (long)n);
        return;
    }

    switch ((menu_msg_type_t)m.type) {
        case MENU_MSG_READY:
            s_peer_ready = true;
            s_peer_color = (color_selection_t)m.color;
            ESP_LOGI(TAG, "Peer READY (%s)", color_name(s_peer_color));
            break;
        case MENU_MSG_CANCEL:
            s_peer_ready = false;
            ESP_LOGI(TAG, "Peer CANCELLED");
            break;
        case MENU_MSG_HELLO:
            ESP_LOGI(TAG, "Peer HELLO");
            break;
        default:
            ESP_LOGW(TAG, "Unknown menu msg: %u", (unsigned)m.type);
            break;
    }

    // Update combined readiness whenever we hear from peer.
    menu->both_ready = (s_local_ready && s_peer_ready);
    if (menu->both_ready) {
        menu->state = MENU_STATE_READY;
    }
}

// ---------- public API ----------
void menu_init(menu_t *menu) {
    if (!menu) return;

    // Start at color selection; you can change to WAITING if you prefer.
    menu->state      = MENU_STATE_COLOR_SELECT;
    menu->color      = COLOR_BLUE;     // default
    menu->both_ready = false;

    s_local_ready = false;
    s_peer_ready  = false;
    s_peer_color  = COLOR_BLUE;

    // Optional: announce presence
    menu_send(MENU_MSG_HELLO, menu->color);
}

void menu_set_color(menu_t *menu, color_selection_t color) {
    if (!menu) return;
    menu->color = color;
    // If you want to auto-resolve color conflicts, you can add logic here.
}

void menu_update(menu_t *menu, uint8_t input) {
    if (!menu) return;

    // Always poll peer first so the UI reflects new info quickly.
    menu_poll_peer(menu);

    switch (menu->state) {
        case MENU_STATE_COLOR_SELECT:
            // Toggle color with any directional press (adjust to taste)
            if (input & (BTN_LEFT | BTN_RIGHT | BTN_UP | BTN_DOWN)) {
                menu->color = (menu->color == COLOR_BLUE) ? COLOR_RED : COLOR_BLUE;
            }
            // Confirm: become locally ready and wait for peer
            if (input & BTN_SELECT) {
                s_local_ready   = true;
                menu->both_ready = (s_local_ready && s_peer_ready);
                menu->state      = menu->both_ready ? MENU_STATE_READY : MENU_STATE_WAITING;
                menu_send(MENU_MSG_READY, menu->color);
            }
            break;

        case MENU_STATE_WAITING:
            // Allow cancel to go back and change color
            if (input & BTN_BACK) {
                s_local_ready   = false;
                menu->both_ready = false;
                menu->state      = MENU_STATE_COLOR_SELECT;
                menu_send(MENU_MSG_CANCEL, menu->color);
            }

            // Peer updates handled in menu_poll_peer(); if both ready, state flips to READY there.
            break;

        case MENU_STATE_READY:
            // If you want to allow un-ready before the game starts:
            if (input & BTN_BACK) {
                s_local_ready   = false;
                menu->both_ready = false;
                menu->state      = MENU_STATE_COLOR_SELECT;
                menu_send(MENU_MSG_CANCEL, menu->color);
            }
            break;

        default:
            break;
    }
}

void menu_draw(menu_t *menu) {
    if (!menu) return;

    // NOTE: Replace WHITE with your actual color constant(s).
    // Keep y spacing generous for your display; adjust x/y as needed.
    int y = 0;

    switch (menu->state) {
        case MENU_STATE_COLOR_SELECT:
            lcd_drawString(0, y,   "SNAKE - MULTIPLAYER", WHITE); y += 12;
            lcd_drawString(0, y,   "Select Color:", WHITE);       y += 12;
            {
                char line[32];
                snprintf(line, sizeof(line), "> %s <", color_name(menu->color));
                lcd_drawString(0, y, line, WHITE);                y += 12;
            }
            lcd_drawString(0, y,   "Left/Right: Toggle", WHITE); y += 12;
            lcd_drawString(0, y,   "A: Ready", WHITE);
            break;

        case MENU_STATE_WAITING:
            lcd_drawString(0, y, "Waiting for peer...", WHITE); y += 12;
            {
                char me[32], peer[32];
                snprintf(me,   sizeof(me),   "You:  READY (%s)", s_local_ready ? color_name(menu->color) : "—");
                snprintf(peer, sizeof(peer), "Peer: %s%s",
                         s_peer_ready ? "READY " : "—",
                         s_peer_ready ? color_name(s_peer_color) : "");
                lcd_drawString(0, y, me, WHITE);   y += 12;
                lcd_drawString(0, y, peer, WHITE); y += 12;
            }
            lcd_drawString(0, y, "B: Cancel", WHITE);
            break;

        case MENU_STATE_READY:
            lcd_drawString(0, y, "Both ready!", WHITE); y += 12;
            {
                char vs[48];
                snprintf(vs, sizeof(vs), "%s vs %s",
                         color_name(menu->color),
                         s_peer_ready ? color_name(s_peer_color) : "?");
                lcd_drawString(0, y, vs, WHITE); y += 12;
            }
            lcd_drawString(0, y, "Start incoming...", WHITE);
            break;

        default:
            lcd_drawString(0, y, "Unknown menu state", WHITE);
            break;
    }
}

bool menu_is_ready(menu_t *menu) {
    if (!menu) return false;
    // Keep the boolean synced with the shadow flags.
    menu->both_ready = (s_local_ready && s_peer_ready);
    return menu->both_ready;
}
