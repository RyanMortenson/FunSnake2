#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_timer.h"

#include "game.h"
#include "snake.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "nav.h"
#include "sound.h"
#include "../audio/c24k_8b/Chomp.h"
#include "../audio/c24k_8b/Crash.h"
#include "config.h"
#include "menu.h"

#include "../sprites/apple.h"
#include "../sprites/bluebody.h"
#include "../sprites/bluedown.h"
#include "../sprites/blueleft.h"
#include "../sprites/blueright.h"
#include "../sprites/blueup.h"
#include "../sprites/cherry.h"
#include "../sprites/goldapple.h"
#include "../sprites/graybody.h"
#include "../sprites/graydown.h"
#include "../sprites/grayleft.h"
#include "../sprites/grayright.h"
#include "../sprites/grayup.h"
#include "../sprites/greenapple.h"
#include "../sprites/greenbody.h"
#include "../sprites/greendown.h"
#include "../sprites/greenleft.h"
#include "../sprites/greenright.h"
#include "../sprites/greenup.h"

#define TOTAL_COLUMNS 20
#define TOTAL_ROWS 15
// Aim for roughly 10 frames per second to avoid overwhelming the LCD
#define GAME_MOVE_PERIOD_US 100000

typedef enum {
    init_st,
    waiting_st,
    playing_st,
    game_over_st,
} game_state_t;

game_state_t currentState;

// Game state variables
static uint8_t my_player_id; 
static bool i_am_host;
static menu_t* g_menu = NULL;  // Global menu pointer
static snake_t snake1;
static snake_t snake2;
static coord_t fruit_x;
static coord_t fruit_y;
static uint8_t peer_snake_dir = 2;  // track peer's snake direction
static uint8_t fruits_eaten = 0;

static uint64_t last_move_time_us = 0;

// Helper to check if a coordinate is occupied by a snake
static bool snake_has_block(const snake_t *snake, coord_t x, coord_t y) {
    const snake_block_t *b = snake->queue.head;
    while (b) {
        if (b->block_x == x && b->block_y == y) {
            return true;
        }
        b = b->next;
    }
    return false;
}

static void clear_block(coord_t x, coord_t y) {
    lcd_fillRect(x * 16, y * 16, 16, 16, BLACK);
}

static void draw_snake_body(const snake_t *snake, coord_t x, coord_t y) {
    const uint16_t *body_bitmap = (snake->player == 1) ? bluebody : graybody;
    coord_t body_w = (snake->player == 1) ? BLUEBODY_W : GRAYBODY_W;
    coord_t body_h = (snake->player == 1) ? BLUEBODY_H : GRAYBODY_H;
    lcd_drawRGBBitmap(x * 16, y * 16, (const color_t *)body_bitmap,
                      body_w, body_h);
}

static void draw_snake_head(const snake_t *snake) {
    const snake_block_t *head = snake->queue.head;
    if (!head) {
        return;
    }

    const uint16_t *head_bitmap = NULL;
    coord_t head_w = BLUEUP_W;
    coord_t head_h = BLUEUP_H;

    if (snake->player == 1) {
        switch (snake->direction) {
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
    } else {
        switch (snake->direction) {
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
    }

    lcd_drawRGBBitmap(head->block_x * 16, head->block_y * 16,
                      (const color_t *)head_bitmap, head_w, head_h);
}

static void draw_fruit(coord_t x, coord_t y) {
    lcd_drawRGBBitmap(x * 16, y * 16, (const color_t *)apple, APPLE_W, APPLE_H);
}

static void draw_scores(void) {
    const int score1 = snake1.queue.size - 3;
    const int score2 = snake2.queue.size - 3;

    lcd_fillRect(0, 0, LCD_W, LCD_CHAR_H * 2, BLACK);

    char snake1_score[23];
    char snake2_score[23];
    sprintf(snake1_score, "Blue Score: %d", score1);
    sprintf(snake2_score, "Red Score: %d", score2);
    lcd_drawString(20, 5, snake1_score, BLUE);
    lcd_drawString(120, 5, snake2_score, RED);
}

static void draw_snake_full(const snake_t *snake) {
    const snake_block_t *b = snake->queue.head;
    if (!b) {
        return;
    }

    draw_snake_head(snake);
    b = b->next;
    while (b) {
        draw_snake_body(snake, b->block_x, b->block_y);
        b = b->next;
    }
}

static void update_snake_render(const snake_t *snake,
                                coord_t prev_head_x,
                                coord_t prev_head_y,
                                coord_t prev_tail_x,
                                coord_t prev_tail_y,
                                bool tail_removed) {
    if (snake->queue.size > 1) {
        draw_snake_body(snake, prev_head_x, prev_head_y);
    }
    draw_snake_head(snake);
    if (tail_removed) {
        clear_block(prev_tail_x, prev_tail_y);
    }
}

//draw UI for wait screen
void draw_wait_screen(){
    lcd_fillScreen(BLACK);

    // TODO: use your LCD text function
    lcd_drawString(30, 40, "SNAKE GAME", BLACK);
    lcd_drawString(20, 80, "Press START to begin", BLACK);
}

//draw game over screen
void draw_game_over_screen() {
    lcd_fillScreen(BLACK);
    draw_scores();

    if (snake1.queue.size > snake2.queue.size) {
        lcd_drawString(80, 100, "BLUE WINS!", BLUE);
    } else if (snake2.queue.size > snake1.queue.size) {
        lcd_drawString(80, 100, "RED WINS!", RED);
    } else {
        lcd_drawString(100, 100, "IT'S A TIE!", WHITE);
    }
    lcd_drawString(100, 130, "GAME OVER", RED);
    lcd_drawString(70, 150, "Press START to restart", WHITE);
}

//draws the board to the screen
void draw_board() {
    lcd_fillScreen(BLACK);

    draw_fruit(fruit_x, fruit_y);
    draw_scores();
    draw_snake_full(&snake1);
    draw_snake_full(&snake2);
    lcd_writeFrame();
}

static bool sound_effects_enabled = true;

static void play_sound_effect(const void *samples,
                              uint32_t sample_count,
                              uint32_t bits_per_sample,
                              uint32_t sample_rate)
{
    if (!sound_effects_enabled) {
        return;
    }

    if (!samples || sample_count == 0 || sample_rate == 0 || bits_per_sample == 0) {
        return;
    }

    if ((bits_per_sample % 8) != 0) {
        return;
    }

    (void)sample_rate;  // Sample rate currently fixed by sound driver

    const size_t sample_bytes = sample_count * (bits_per_sample / 8U);
    sound_start(samples, sample_bytes, false);
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

// Add this function to set menu from main
void game_set_menu(menu_t* menu) {
    g_menu = menu;
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
            // Start only after both players have confirmed START on the menu
            if (g_menu && g_menu->both_ready) {
                // Assign player ID based on color
                my_player_id = (g_menu->color == COLOR_BLUE) ? 1 : 2;
                i_am_host = (my_player_id == 1);
                
                
                snake_init(&snake1, 1);
                snake_init(&snake2, 2);
                spawn_fruit();
                lcd_frameEnable();

                last_move_time_us = esp_timer_get_time();
                draw_board();
                currentState = playing_st;
            }
            break;

        case playing_st:
            const uint64_t now_us = esp_timer_get_time();
            if ((now_us - last_move_time_us) >= (GAME_MOVE_PERIOD_US - 1000*fruits_eaten)) {
                const coord_t prev_fruit_x = fruit_x;
                const coord_t prev_fruit_y = fruit_y;
                bool fruit_moved = false;

                const coord_t p1_prev_head_x = snake1.queue.head->block_x;
                const coord_t p1_prev_head_y = snake1.queue.head->block_y;
                const coord_t p1_prev_tail_x = snake1.queue.tail->block_x;
                const coord_t p1_prev_tail_y = snake1.queue.tail->block_y;

                const coord_t p2_prev_head_x = snake2.queue.head->block_x;
                const coord_t p2_prev_head_y = snake2.queue.head->block_y;
                const coord_t p2_prev_tail_x = snake2.queue.tail->block_x;
                const coord_t p2_prev_tail_y = snake2.queue.tail->block_y;

                // --- POLL PEER FOR UPDATES ---
                game_msg_t* peer_msg = com_recv_game();
                if (peer_msg) {
                    if (peer_msg->type == GAME_MSG_MOVE) {
                        peer_snake_dir = peer_msg->dir;
                        snake_t* peer_snake = (my_player_id == 1 ? &snake2 : &snake1);
                        snake_change_direction(peer_snake, peer_msg->dir);
                    } else if (peer_msg->type == GAME_MSG_FRUIT) {
                        fruit_x = peer_msg->x;
                        fruit_y = peer_msg->y;
                        fruit_moved = true;
                    }
                }

                // --- UPDATE JOYSTICK NAVIGATOR ---
                nav_tick();

                int8_t r, c;
                nav_get_loc(&r, &c);

                static int8_t prev_r = 10;  // CONFIG_BOARD_R/2
                static int8_t prev_c = 10;  // CONFIG_BOARD_C/2

                int dr = r - prev_r;
                int dc = c - prev_c;

                snake_t* mySnake = (my_player_id == 1 ? &snake1 : &snake2);
                uint8_t new_dir = mySnake->direction;  // default: keep direction

                if (dc > 0) {
                    new_dir = RIGHT;
                    snake_change_direction(mySnake, RIGHT);
                } else if (dc < 0) {
                    new_dir = LEFT;
                    snake_change_direction(mySnake, LEFT);
                }
                if (dr > 0) {
                    new_dir = DOWN;
                    snake_change_direction(mySnake, DOWN);
                } else if (dr < 0) {
                    new_dir = UP;
                    snake_change_direction(mySnake, UP);
                }

                prev_r = r;
                prev_c = c;

                // Send my direction to peer
                com_send_move(new_dir, mySnake->queue.head->block_x, mySnake->queue.head->block_y);

                // Move snakes
                bool p1_ate = (snake1.queue.head->block_x == fruit_x &&
                            snake1.queue.head->block_y == fruit_y);

                bool p2_ate = (snake2.queue.head->block_x == fruit_x &&
                            snake2.queue.head->block_y == fruit_y);

                snake_move(&snake1, p1_ate);
                snake_move(&snake2, p2_ate);

                if (i_am_host && (p1_ate || p2_ate)) {
                    fruit_moved = true;
                    spawn_fruit();
                    fruits_eaten++;
                    play_sound_effect(chomp_samples,
                                    CHOMP_SAMPLES,
                                    CHOMP_BITS_PER_SAMPLE,
                                    CHOMP_SAMPLE_RATE);
                    com_send_fruit(fruit_x, fruit_y);
                }

                // COLLISION DETECTION
                if (hit_wall(&snake1) || hit_wall(&snake2) ||
                    self_collision(&snake1) || self_collision(&snake2) ||
                    snake_collision(&snake1, &snake2) ||
                    snake_collision(&snake2, &snake1)) {

                    play_sound_effect(sound_effect_car_crash_394903,
                                    SOUND_EFFECT_CAR_CRASH_394903_SAMPLES,
                                    SOUND_EFFECT_CAR_CRASH_394903_BITS_PER_SAMPLE,
                                    SOUND_EFFECT_CAR_CRASH_394903_SAMPLE_RATE);
                    currentState = game_over_st;
                    draw_game_over_screen();
                    if (g_menu) {
                        menu_reset_sync(g_menu, true);
                    }
                } else {
                    update_snake_render(&snake1, p1_prev_head_x, p1_prev_head_y,
                                        p1_prev_tail_x, p1_prev_tail_y, !p1_ate);
                    update_snake_render(&snake2, p2_prev_head_x, p2_prev_head_y,
                                        p2_prev_tail_x, p2_prev_tail_y, !p2_ate);

                    if (fruit_moved) {
                        if (!snake_has_block(&snake1, prev_fruit_x, prev_fruit_y) &&
                            !snake_has_block(&snake2, prev_fruit_x, prev_fruit_y)) {
                            clear_block(prev_fruit_x, prev_fruit_y);
                        }
                        draw_fruit(fruit_x, fruit_y);
                    }

                    draw_scores();
                    lcd_writeFrame();
                    last_move_time_us = now_us;
                }
            }
            break;

        case game_over_st:
            // After a round ends, wait for the menu handshake to complete again
            // so both players press START before restarting.
            lcd_frameDisable();
            if (g_menu && g_menu->both_ready) {
                currentState = waiting_st;
            }
            break;
    }
}

