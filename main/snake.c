#include "snake.h"
#include <stdlib.h>

#define START_Y 8
#define P1_START_X 3
#define P2_START_X 18
//add to snakes head
void snake_push_head(snake_queue_t *q, coord_t x, coord_t y){
    snake_block_t *node = malloc(sizeof(*node));
    node->block_x = x;
    node->block_y = y;
    node->next = q->head;

    q->head = node;

    if (q->tail == NULL){
        q->tail = node;
    }

    q->size++;
}

//initialize snake
void snake_init(snake_t *snake, uint8_t player){
    snake->queue.head = NULL;
    snake->queue.tail = NULL;
    snake->queue.size = 0;
    snake->player = player;

    if (player == 1){
        snake->direction = SNAKE_DIR_RIGHT;
        snake_push_head(&snake->queue, P1_START_X-2, START_Y);  // tail
        snake_push_head(&snake->queue, P1_START_X-1, START_Y);
        snake_push_head(&snake->queue, P1_START_X, START_Y);
    } else if (player == 2){
        snake->direction = SNAKE_DIR_LEFT;
        snake_push_head(&snake->queue, P2_START_X+2, START_Y);  // tail
        snake_push_head(&snake->queue, P2_START_X+1, START_Y);
        snake_push_head(&snake->queue, P2_START_X, START_Y);
    }
}

//removes the tail piece of the snake
void snake_pop_tail(snake_queue_t *q){
    if (q->tail == NULL){
        return;
    }

    if (q->head == q->tail){
        free(q->tail);
        q->head = q->tail = NULL;
        q->size = 0;
        return;
    }

    snake_block_t *cur = q->head;
    while (cur->next != q->tail){
        cur = cur->next;
    }
    free(q->tail);
    q->tail = cur;
    q->tail->next = NULL;
    q->size--;
}

//move snake
void snake_move(snake_t *snake, bool ate_fruit){
    if (snake->direction == SNAKE_DIR_UP){
        snake_push_head(&snake->queue, snake->queue.head->block_x, snake->queue.head->block_y -1);
    } else if (snake->direction == SNAKE_DIR_DOWN){
        snake_push_head(&snake->queue, snake->queue.head->block_x, snake->queue.head->block_y +1);
    } else if (snake->direction == SNAKE_DIR_RIGHT){
        snake_push_head(&snake->queue, snake->queue.head->block_x + 1, snake->queue.head->block_y);
    } else if (snake->direction == SNAKE_DIR_LEFT){
        snake_push_head(&snake->queue, snake->queue.head->block_x - 1, snake->queue.head->block_y);
    }
    if (!ate_fruit){
        snake_pop_tail(&snake->queue);
    }
}

//changes snake direction
void snake_change_direction(snake_t *snake, snake_direction_t new_direction){
    if ((snake->direction == SNAKE_DIR_UP && new_direction == SNAKE_DIR_DOWN) ||
        (snake->direction == SNAKE_DIR_DOWN && new_direction == SNAKE_DIR_UP) ||
        (snake->direction == SNAKE_DIR_LEFT && new_direction == SNAKE_DIR_RIGHT) ||
        (snake->direction == SNAKE_DIR_RIGHT && new_direction == SNAKE_DIR_LEFT)) {
        return;
    }
    snake->direction = new_direction;
}