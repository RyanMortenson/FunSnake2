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

#endif // MENU_H