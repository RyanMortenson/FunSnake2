// com.c — ultra-minimal color picker FSM
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "menu.h"
#include "lcd.h"
#include "net.h"

#define MESS_FONT_SZ 1

#define MESS_X (LCD_CHAR_W*MESS_FONT_SZ)
#define MESS_Y (LCD_H-MESS_H)
#define MESS_W (LCD_W-LCD_CHAR_W*MESS_FONT_SZ*2)
#define MESS_H (LCD_CHAR_H*MESS_FONT_SZ)

#define GROUP_ID 1
#define GROUP_WAIT 4000 // ms
#define SEND_COUNT 20
#define SEND_DELAY 1000
#ifndef WHITE
#define WHITE 0xFFFF
#endif

void graphics_drawMessage(const char *str, color_t color, color_t bg)
{
	lcd_fillRect(MESS_X, MESS_Y, MESS_W, MESS_H, bg);
	lcd_setFontSize(MESS_FONT_SZ);
	lcd_drawString(MESS_X, MESS_Y, str, color);
}

int32_t com_init(void){
    int32_t ret = net_init();
    if (ret) {
		return -1;
	}
    net_group_open(GROUP_ID);
    graphics_drawMessage("Waiting to join group...", CYAN, rgb565(0, 16, 42));
    vTaskDelay(pdMS_TO_TICKS(GROUP_WAIT));
    net_group_close();
    return 0;
}

// Free resources used for communication.
// Return zero if successful, or non-zero otherwise.
int32_t com_deinit(void){
     return net_deinit();
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size){
    if (!buf || size == 0){
        return -1;
    }
    return net_send(NULL, buf, size, 0); 
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size){
    if (!buf || size == 0){
        return -1;
    }
    uint8_t src[NET_ALEN];
    return net_recv(src, buf, size, 0); 
}

// Game protocol helpers
void com_send_move(uint8_t dir, uint8_t x, uint8_t y) {
    game_msg_t m = { GAME_MSG_MOVE, dir, x, y };
    com_write(&m, sizeof(m));
}

void com_send_fruit(uint8_t x, uint8_t y) {
    game_msg_t m = { GAME_MSG_FRUIT, 0, x, y };
    com_write(&m, sizeof(m));
}

// Returns NULL if no message, or pointer to game_msg_t if received
game_msg_t* com_recv_game(void) {
    static game_msg_t m;
    int32_t n = com_read(&m, sizeof(m));
    if (n != (int32_t)sizeof(m)) return NULL;
    return &m;
}
