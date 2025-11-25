#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // rand

#include "game.h"
#include "snake.h"
#include "hw.gc.h"
#include "lcd.h"
#include "pin.h"
#include "nav.h"
#include "sound.h"
#include "sprites/all_sprites.h"
#include "audio/Chomp.h"
#include "audio/Crash.h"
#include "audio/LevelUp.h"

#define TOTAL_COLUMNS HW_LCD_W/16  //20 Total
#define TOTAL_ROWS HW_LCD_H/16     //15 total

typedef enum {
    init_st,
    waiting_st,
    playing_st,
    game_over_st,
} game_state_t;

game_state_t currentState;

// Game state variables
static snake_t snake1;
static snake_t snake2;
static coord_t fruit_x;
static coord_t fruit_y;

//draw UI for wait screen
void draw_wait_screen(){
    lcd_clear();

    // TODO: use your LCD text function
    lcd_draw_text(30, 40, "SNAKE GAME", LCD_WHITE);
    lcd_draw_text(20, 80, "Press START to begin", LCD_WHITE);
}

//draw game over screen
void draw_game_over_screen() {
    lcd_clear();
    lcd_draw_text(30, 60, "GAME OVER", LCD_RED);
    lcd_draw_text(10, 100, "Press START to restart", LCD_WHITE);
}

//draws the board to the screen
void draw_board() {
    lcd_clear();

    // Draw fruit
    lcd_drawRGBBitmap(fruit_x * 16, fruit_y * 16, (const color_t *)apple, APPLE_W, APPLE_H);

    // Draw snake 1
    snake_block_t *b = snake1.queue.head;
    if (b) {
        const uint16_t *head_bitmap = NULL;
        coord_t head_w = BLUEUP_W;
        coord_t head_h = BLUEUP_H;

        switch (snake1.direction) {
            case SNAKE_DIR_UP:
                head_bitmap = blueup;
                head_w = BLUEUP_W;
                head_h = BLUEUP_H;
                break;
            case SNAKE_DIR_RIGHT:
                head_bitmap = blueright;
                head_w = BLUERIGHT_W;
                head_h = BLUERIGHT_H;
                break;
            case SNAKE_DIR_DOWN:
                head_bitmap = bluedown;
                head_w = BLUEDOWN_W;
                head_h = BLUEDOWN_H;
                break;
            case SNAKE_DIR_LEFT:
            default:
                head_bitmap = blueleft;
                head_w = BLUELEFT_W;
                head_h = BLUELEFT_H;
                break;
        }

        lcd_drawRGBBitmap(b->block_x * 16, b->block_y * 16, (const color_t *)head_bitmap, head_w, head_h);
        b = b->next;

        while (b) {
            lcd_drawRGBBitmap(b->block_x * 16, b->block_y * 16, (const color_t *)bluebody, BLUEBODY_W, BLUEBODY_H);
            b = b->next;
        }
    }

    // Draw snake 2
    b = snake2.queue.head;
    if (b) {
        const uint16_t *head_bitmap = NULL;
        coord_t head_w = GRAYUP_W;
        coord_t head_h = GRAYUP_H;

        switch (snake2.direction) {
            case SNAKE_DIR_UP:
                head_bitmap = grayup;
                head_w = GRAYUP_W;
                head_h = GRAYUP_H;
                break;
            case SNAKE_DIR_RIGHT:
                head_bitmap = grayright;
                head_w = GRAYRIGHT_W;
                head_h = GRAYRIGHT_H;
                break;
            case SNAKE_DIR_DOWN:
                head_bitmap = graydown;
                head_w = GRAYDOWN_W;
                head_h = GRAYDOWN_H;
                break;
            case SNAKE_DIR_LEFT:
            default:
                head_bitmap = grayleft;
                head_w = GRAYLEFT_W;
                head_h = GRAYLEFT_H;
                break;
        }

        lcd_drawRGBBitmap(b->block_x * 16, b->block_y * 16, (const color_t *)head_bitmap, head_w, head_h);
        b = b->next;

        while (b) {
            lcd_drawRGBBitmap(b->block_x * 16, b->block_y * 16, (const color_t *)graybody, GRAYBODY_W, GRAYBODY_H);
            b = b->next;
        }
    }
}

static void play_sound_effect(const int16_t *samples, uint32_t sample_count, uint32_t sample_rate) {
    if (samples && sample_count > 0 && sample_rate > 0) {
        sound_play(samples, sample_count, sample_rate);
    }
}

//chooses random location on board to put fruit
void spawn_fruit() {
    fruit_x = rand() % TOTAL_COLUMNS;
    fruit_y = rand() % TOTAL_ROWS;
}

//checks if snake has hit the wall
bool hit_wall(snake_t *s) {
    int x = s->queue.head->block_x;
    int y = s->queue.head->block_y;
    return (x < 0 || x >= TOTAL_COLUMNS || y < 0 || y >= TOTAL_ROWS);
}

//checks if the snake has hit itself
bool self_collision(snake_t *s) {
    snake_block_t *head = s->queue.head;
    snake_block_t *cur = head->next;

    while (cur) {
        if (cur->block_x == head->block_x && cur->block_y == head->block_y) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

//checks if snake has hit other snake
bool snake_collision(snake_t *a, snake_t *b) {
    snake_block_t *head = a->queue.head;
    snake_block_t *cur = b->queue.head;

    while (cur) {
        if (cur->block_x == head->block_x && cur->block_y == head->block_y) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

// Initialize the game control logic.
void game_init(void){
    currentState = init_st;

}

// Update the game control logic.
// detects collisions, and updates statistics.
void game_tick(void){
    //FSM for the game
    switch(currentState){
        case init_st:
            nav_init(50);
            draw_wait_screen();
            currentState = waiting_st;
            break;

        case waiting_st:
            if (!pin_get_level(HW_BTN_START)) {
                snake_init(&snake1, 1);
                snake_init(&snake2, 2);
                spawn_fruit();
                play_sound_effect(ESM_Ambient_Game_Level_Up_Soft_Tone_1_Upgrade_Unlock_Bonus_Arcade_Fun,
                                  ESM_AMBIENT_GAME_LEVEL_UP_SOFT_TONE_1_UPGRADE_UNLOCK_BONUS_ARCADE_FUN_SAMPLES,
                                  ESM_AMBIENT_GAME_LEVEL_UP_SOFT_TONE_1_UPGRADE_UNLOCK_BONUS_ARCADE_FUN_SAMPLE_RATE);
                currentState = playing_st;
            }
            break;

        case playing_st:
            // --- UPDATE JOYSTICK NAVIGATOR ---
            nav_tick();

            int8_t r, c;
            nav_get_loc(&r, &c);

            static int8_t prev_r = GRID_R/2;
            static int8_t prev_c = GRID_C/2;

            int dr = r - prev_r;
            int dc = c - prev_c;

            if (dc > 0) snake_change_direction(&snake1, SNAKE_DIR_RIGHT);
            else if (dc < 0) snake_change_direction(&snake1, SNAKE_DIR_LEFT);
            if (dr > 0) snake_change_direction(&snake1, SNAKE_DIR_DOWN);
            else if (dr < 0) snake_change_direction(&snake1, SNAKE_DIR_UP);

            prev_r = r;
            prev_c = c;
            // TODO: read controls to change directions
            // e.g. snake_change_direction(&snake1, DIR_UP);

            // Move snakes
            bool p1_ate = (snake1.queue.head->block_x == fruit_x &&
                        snake1.queue.head->block_y == fruit_y);

            bool p2_ate = (snake2.queue.head->block_x == fruit_x &&
                        snake2.queue.head->block_y == fruit_y);

            snake_move(&snake1, p1_ate);
            snake_move(&snake2, p2_ate);

            if (p1_ate || p2_ate) {
                spawn_fruit();
                play_sound_effect(PFS2_Carrot_Chomp_7,
                                  PFS2_CARROT_CHOMP_7_SAMPLES,
                                  PFS2_CARROT_CHOMP_7_SAMPLE_RATE);
            }

            // COLLISION DETECTION
            if (hit_wall(&snake1) || hit_wall(&snake2) ||
                self_collision(&snake1) || self_collision(&snake2) ||
                snake_collision(&snake1, &snake2) ||
                snake_collision(&snake2, &snake1)) {

                play_sound_effect(CrashAuto_BW_17108,
                                  CRASHAUTO_BW_17108_SAMPLES,
                                  CRASHAUTO_BW_17108_SAMPLE_RATE);
                currentState = game_over_st;
                break;
            }

            // Draw updated game state
            draw_board();
            break;

        case game_over_st:
            draw_game_over_screen();

            if (hw_buttons_start_pressed()) {  // restart
                currentState = init_st;
            }
            break;
    }

}
