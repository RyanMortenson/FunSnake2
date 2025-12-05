#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

// Menu states
typedef enum {
    MENU_STATE_WAITING,
    MENU_STATE_COLOR_SELECT,
    MENU_STATE_READY
} menu_state_t;

typedef enum {
    COLOR_BLUE,
    COLOR_RED
} color_selection_t;

// Menu structure to track current state
typedef struct {
    menu_state_t state;
    color_selection_t color;
    bool both_ready;
} menu_t;

// Function declarations
void menu_init(menu_t *menu);
void menu_update(menu_t *menu, uint8_t input);
void menu_draw(menu_t *menu);
void menu_set_color(menu_t *menu, color_selection_t color);
bool menu_is_ready(menu_t *menu);
void menu_reset_sync(menu_t *menu, bool notify_peer);

// Game protocol messages
typedef enum {
    MENU_MSG_READY  = 1,
    MENU_MSG_CANCEL = 2,
    MENU_MSG_START  = 3,
    MENU_MSG_START_CANCEL = 4,
    GAME_MSG_MOVE   = 10,   // Snake direction + position
    GAME_MSG_FRUIT  = 11    // Fruit spawn location
} msg_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;           // msg_type_t
    uint8_t dir;            // direction (UP/DOWN/LEFT/RIGHT)
    uint8_t x, y;           // snake head position or fruit position
} game_msg_t;

// Game protocol helpers
void com_send_move(uint8_t dir, uint8_t x, uint8_t y);
void com_send_fruit(uint8_t x, uint8_t y);
game_msg_t* com_recv_game(void);

#endif // MENU_H