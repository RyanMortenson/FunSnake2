#include "freertos/FreeRTOS.h"
#include "hw.h"
#include "lcd.h"
#include "net.h"
#include "com.h"
#include "esp_log.h"

#define GROUP_ID 1
#define GROUP_WAIT 4000 // ms
#define MAX_STR 32

static const char *TAG = "test_net";

typedef struct {
	uint32_t i;
	char str[MAX_STR];
} event_t;

// Receive packets and display contents.


//initialize communication
int32_t com_init(void){
    int32_t ret;

	ESP_LOGI(TAG, "app_main");

	// Initialize network.
	ret = net_init();
  

    // Open group registration.
    ret = net_group_open(GROUP_ID);
	lcd_drawString(0,0,"Wait for devices to join group...", WHITE);
    vTaskDelay(pdMS_TO_TICKS(GROUP_WAIT));
	ret = net_group_close();
    return ret;
}

// deinitialize communication
int32_t com_deinit(void){
    return net_deinit();
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size){
    return net_send(NULL, buf, size, 0);
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size){
    uint8_t src[NET_ALEN];
    return net_recv(src, buf, size, 0);
}
