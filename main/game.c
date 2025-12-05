#include <stdbool.h>
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
#include "../audio/Chomp.h"
#include "../audio/Crash.h"
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

static uint64_t last_move_time_us = 0;

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
    if (snake1.queue.size > snake2.queue.size) {
        lcd_drawString(80, 100, "BLUE WINS!", BLUE);
    } else if (snake2.queue.size > snake1.queue.size) {
        lcd_drawString(80, 100, "RED WINS!", RED);
    } else {
        lcd_drawString(100, 100, "IT'S A TIE!", WHITE);
    }
    lcd_drawString(100, 130, "GAME OVER", RED);
    lcd_drawString(100, 150, "Press START to restart", WHITE);
}

//draws the board to the screen
void draw_board() {
    lcd_fillScreen(BLACK);

    // Draw fruit
    lcd_drawRGBBitmap(fruit_x * 16, fruit_y * 16, (const color_t *)apple, APPLE_W, APPLE_H);
    char snake1_score[23];
    char snake2_score[23];
    sprintf(snake1_score, "Blue Score: %d", snake1.queue.size - 3);
    sprintf(snake2_score, "Red Score: %d", snake2.queue.size - 3);
    lcd_drawString(20, 5, snake1_score, BLUE);
    lcd_drawString(120, 5, snake2_score, RED);
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
    lcd_writeFrame();
}

static bool sound_effects_enabled = false;

static void play_sound_effect(const int16_t *samples, uint32_t sample_count, uint32_t sample_rate) {
    if (!sound_effects_enabled) {
        return;  // Temporarily muted per request
    }

    if (samples && sample_count > 0 && sample_rate > 0) {
        const size_t sample_bytes = sample_count * sizeof(samples[0]);
        int16_t *scaled = malloc(sample_bytes);

        if (scaled) {
            // Keep effects at a gentler level to avoid blasting the speaker.
            const int8_t attenuation = 8;
            for (uint32_t i = 0; i < sample_count; ++i) {
                scaled[i] = samples[i] / attenuation;
            }
            sound_start(scaled, sample_bytes, true);
            free(scaled);
        } else {
            // Fall back to original buffer if allocation fails
            sound_start(samples, sample_bytes, true);
        }
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
<<<<<<< ours
                lcd_frameEnable();
=======
                last_move_time_us = esp_timer_get_time();
                draw_board();
>>>>>>> theirs
                currentState = playing_st;
            }
            break;

        case playing_st:
            const uint64_t now_us = esp_timer_get_time();
            if ((now_us - last_move_time_us) >= GAME_MOVE_PERIOD_US) {
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
                    spawn_fruit();
                    play_sound_effect(PFS2_Carrot_Chomp_7,
                                    PFS2_CARROT_CHOMP_7_SAMPLES,
                                    PFS2_CARROT_CHOMP_7_SAMPLE_RATE);
                    com_send_fruit(fruit_x, fruit_y);
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
                    draw_game_over_screen();
                    if (g_menu) {
                        menu_reset_sync(g_menu, true);
                    }
                } else {
                    draw_board();
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

