#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"


#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"
#include "config.h"
#include "com.h"
#include "game.h"
#include "menu.h"

#include "esp_heap_caps.h"


static const char *TAG = "lab07";


// The update period as an integer in ms
#define PER_MS ((uint32_t)(CONFIG_GAME_TIMER_PERIOD*1000))
#define TIME_OUT 500 // ms


#define SOUND_SAMPLE_RATE 32000
#define CURSOR_SZ 7 // Cursor size (width & height) in pixels


//helps initialize cursor
#define CHK_RET(x) ({                                           \
       int32_t ret_val = (x);                                  \
       if (ret_val != 0) {                                     \
           ESP_LOGE(TAG, "FAIL: return %ld, %s", ret_val, #x); \
       }                                                       \
       ret_val;                                                \
   })




TimerHandle_t update_timer; // Declare timer handle for update callback


volatile bool interrupt_flag;


uint32_t isr_triggered_count;
uint32_t isr_handled_count;


// Interrupt handler for game - use flag method
void update() {
   interrupt_flag = true;
   isr_triggered_count++;
}

// WEIRD CHAT THING::

static size_t lcd_framebuffer_bytes(void) {
   // The panel runs in RGB565 mode; the driver expects two bytes per pixel.
   return (size_t)LCD_W * (size_t)LCD_H * sizeof(uint16_t);
}

static bool lcd_enable_framebuffer_if_possible(void) {
   const size_t fb_needed = lcd_framebuffer_bytes();
   size_t largest_free = heap_caps_get_largest_free_block(MALLOC_CAP_DMA);

   if (largest_free < fb_needed) {
       ESP_LOGW(TAG, "Frame buffer skipped: need ~%d bytes, largest block %d bytes", (int)fb_needed, (int)largest_free);
       return false;
   }

   lcd_frameEnable();
   return true;
}




// Draw the cursor on the screen
void cursor(coord_t x, coord_t y, color_t color)
{
   coord_t s2 = CURSOR_SZ >> 1; // size div 2
   lcd_drawHLine(x-s2, y,    CURSOR_SZ, color);
   lcd_drawVLine(x,    y-s2, CURSOR_SZ, color);
}


// Main application
void app_main(void){
   ESP_LOGI(TAG, "Starting");


   // ISR flag and counts
   interrupt_flag = false;
   isr_triggered_count = 0;
   isr_handled_count = 0;


   // Initialization
   lcd_init();
   lcd_enable_framebuffer_if_possible(); // Enable frame buffer mode
   lcd_fillScreen(CONFIG_COLOR_BACKGROUND);
   CHK_RET(cursor_init(PER_MS));
   sound_init(SOUND_SAMPLE_RATE);
   CHK_RET(com_init());
   game_init();
   
   menu_t menu;
   menu_init(&menu);
   game_set_menu(&menu);  // Pass menu to game


   // Configure I/O pins for buttons
   pin_reset(HW_BTN_A);
   pin_input(HW_BTN_A, true);
   pin_reset(HW_BTN_B);
   pin_input(HW_BTN_B, true);
   pin_reset(HW_BTN_MENU);
   pin_input(HW_BTN_MENU, true);
   pin_reset(HW_BTN_OPTION);
   pin_input(HW_BTN_OPTION, true);
   pin_reset(HW_BTN_SELECT);
   pin_input(HW_BTN_SELECT, true);
   pin_reset(HW_BTN_START);
   pin_input(HW_BTN_START, true);


   // Initialize update timer
   update_timer = xTimerCreate(
       "update_timer",        // Text name for the timer.
       pdMS_TO_TICKS(PER_MS), // The timer period in ticks.
       pdTRUE,                // Auto-reload the timer when it expires.
       NULL,                  // No need for a timer ID.
       update                 // Function called when timer expires.
   );
   if (update_timer == NULL) {
       ESP_LOGE(TAG, "Error creating update timer");
       return;
   }
   if (xTimerStart(update_timer, pdMS_TO_TICKS(TIME_OUT)) != pdPASS) {
       ESP_LOGE(TAG, "Error starting update timer");
       return;
   }


   // Main game loop
   uint64_t t1, t2, tmax = 0; // For hardware timer values
   coord_t x, y; // For cursor position
   while (pin_get_level(HW_BTN_MENU)) // while MENU button not pressed
   {
       while (!interrupt_flag) ;
       t1 = esp_timer_get_time();
       interrupt_flag = false;
       isr_handled_count++;


       #ifndef CONFIG_ERASE
       #endif // CONFIG_ERASE
       
       // Read button inputs (map to bitmask for menu)
       uint8_t input = 0;
        if (!pin_get_level(HW_BTN_A)) input |= (1u << 4);       // SELECT
        if (!pin_get_level(HW_BTN_B)) input |= (1u << 5);       // BACK
        if (!pin_get_level(HW_BTN_OPTION)) input |= (1u << 0);  // LEFT
        if (!pin_get_level(HW_BTN_START)) input |= (1u << 1);   // RIGHT
       
       // Menu phase: show menu until both players ready
       if (!menu_is_ready(&menu)) {
           menu_update(&menu, input);
           menu_draw(&menu);
       } else {
           // Game phase: run the game
           game_tick();
       }
       
       cursor_tick();
       cursor_get_pos(&x, &y);
       t2 = esp_timer_get_time() - t1;
       if (t2 > tmax) tmax = t2;
   }
   printf("Handled %lu of %lu interrupts\n", isr_handled_count, isr_triggered_count);
   printf("WCET us:%llu\n", tmax);
   sound_deinit();
}
