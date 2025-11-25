#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "snake.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "nav.h"
#include "config.h"
#include "menu.h"

#define TOTAL_COLUMNS 20
#define TOTAL_ROWS 15

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
static uint8_t peer_snake_dir = RIGHT;  // track peer's snake direction

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
    // TODO: replace with your draw-square function
    lcd_draw_rect(fruit_x * 16, fruit_y * 16, 16, 16, LCD_RED);

    // Draw snake 1
    snake_block_t *b = snake1.queue.head;
    while (b) {
        lcd_draw_rect(b->block_x * 16, b->block_y * 16, 16, 16, LCD_GREEN);
        b = b->next;
    }

    // Draw snake 2
    b = snake2.queue.head;
    while (b) {
        lcd_draw_rect(b->block_x * 16, b->block_y * 16, 16, 16, LCD_BLUE);
        b = b->next;
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
            //UPDATE PLAYER ID AND HOST HERE
            if (!pin_get_level(HW_BTN_START)) {
                // Assign player ID based on color
                my_player_id = (g_menu && g_menu->color == COLOR_BLUE) ? 1 : 2;
                i_am_host = true;
                
                snake_init(&snake1, 1);
                snake_init(&snake2, 2);
                spawn_fruit();
                currentState = playing_st;
            }
            break;

        case playing_st:
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
                com_send_fruit(fruit_x, fruit_y);
            }

            // COLLISION DETECTION
            if (hit_wall(&snake1) || hit_wall(&snake2) ||
                self_collision(&snake1) || self_collision(&snake2) ||
                snake_collision(&snake1, &snake2) ||
                snake_collision(&snake2, &snake1)) {

                currentState = game_over_st;
                break;
            }

            // Draw updated game state
            draw_board();
            break;

        case game_over_st:
            draw_game_over_screen();

            if (!pin_get_level(HW_BTN_START)) {  // restart
                currentState = init_st;
            }
            break;
    }
    
}