#ifndef SNAKE_H_
#define SNAKE_H_

#include <stdbool.h>
#include <stdint.h>

#include "lcd.h"

#define SNAKE_MAX 256

typedef struct snake_block_t {
    coord_t block_x;
    coord_t block_y;
    struct snake_block_t *next;
} snake_block_t;

typedef struct snake_queue_t {
    snake_block_t *head;  // front of queue (snake head)
    snake_block_t *tail;  // back of queue (snake tail)
    uint16_t size;
} snake_queue_t;

typedef enum {
    SNAKE_DIR_UP = 1,
    SNAKE_DIR_RIGHT = 2,
    SNAKE_DIR_DOWN = 3,
    SNAKE_DIR_LEFT = 4,
} snake_direction_t;

// This struct contains all information about a snake.
typedef struct {
    // Current state. The 'enum' is defined in your snake.c file.
    int32_t currentState;

    //queue for snake
    snake_queue_t queue;

    //direction the snake is headed(same as joystick)
    //up:1
    //right:2
    //down:3
    //left:4
    snake_direction_t direction;

    //player number of snake
    uint8_t player;
} snake_t;

//add to the front of the queue
void snake_push_head(snake_queue_t *q, coord_t x, coord_t y);

//initialize snake variables
void snake_init(snake_t *snake, uint8_t player);

//removes the tail piece of the snake
void snake_pop_tail(snake_queue_t *q);

//function that will move the snake one block forward
void snake_move(snake_t *snake, bool ate_fruit);

//changed snakes direction
void snake_change_direction(snake_t *snake, snake_direction_t new_direction);

#endif  // SNAKE_H_
