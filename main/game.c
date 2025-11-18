#include <stdio.h>
#include <stdlib.h> // rand

#include "game.h"
#include "hw.gc.h"
#include "lcd.h"

#define TOTAL_COLUMNS HW_LCD_W/5
#define TOTAL_ROWS HW_LCD_H/5

typedef enum {
    init_st,
    waiting_st,
    playing_st,
    game_over_st,
} game_state_t;

game_state_t currentState;

//draw UI for wait screen
void draw_wait_screen(){
    
}

// Initialize the game control logic.
void game_init(void){
    currentState = init_st;

}

// Update the game control logic.
// detects collisions, and updates statistics.
void game_tick(void){
    
}