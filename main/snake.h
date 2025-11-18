#define SNAKE_MAX 256


typedef struct {
	coord_t block_x;
	coord_t block_y;
} snake_block_t;


// This struct contains all information about a snake.
typedef struct {
	// Current state. The 'enum' is defined in your snake.c file.
	int32_t currentState;

	// Array of snake body blocks
	snake_block_t body[SNAKE_MAX];

    // Location of the snek head
    snake_block_t head;

	// The total length of the snake
	int32_t total_length;
} snake_t;
