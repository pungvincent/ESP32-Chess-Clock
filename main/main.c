#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"

// OLED Display component
#include "ssd1306.h"
#include "font8x8_basic.h"

#define tag "SSD1306"

//OLED Display Config
SSD1306_t dev;
int center, top, bottom;
char lineChar[20];

/*
 You have to set this config value with menuconfig
 CONFIG_INTERFACE

 for i2c
 CONFIG_MODEL
 CONFIG_SDA_GPIO
 CONFIG_SCL_GPIO
 CONFIG_RESET_GPIO

 for SPI
 CONFIG_CS_GPIO
 CONFIG_DC_GPIO
 CONFIG_RESET_GPIO
*/

// GPIO for buttons
#define BUTTON_PLAYER1_GPIO 2
#define BUTTON_PLAYER2_GPIO 4
#define BUTTON_RESET_GPIO 16
#define BUTTON_PAUSE_GPIO 17

// Interruption
#define ESP_INR_FLAG_DEFAULT 0

// Initial time for each player (3 minutes = 180 seconds)
#define PLAYER_TIME 180
static int player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
static int player2_time = PLAYER_TIME*100;  //Convert to hundredth of a second

//Turn flag
static bool player1_turn = false;
static bool player2_turn = false;  

// Semaphore to synchronize tasks
SemaphoreHandle_t xSemaphore = NULL;

// Timer variables
esp_timer_handle_t timer_handle_player1;
esp_timer_handle_t timer_handle_player2;
esp_timer_handle_t timer_handle_display;

//Track Variables
bool player1_timer_running = false;  // Variable to track if player 1's timer is running
bool player2_timer_running = false;  // Variable to track if player 2's timer is running


// Function to display the remaining time
void print_time() {
        // Affichage dans le terminal pour débogage
        printf("Player 1: %02d:%02d:%02d  Player 2: %02d:%02d:%02d\n", 
            player1_time/(60*100), player1_time/100%60, player1_time%100, 
            player2_time/(60*100), player2_time/100%60, player2_time%100);
}

// Timer callbacks functions (decrements time each seconds)
void player1_timer(void* arg) {
    if (player1_time > 0) player1_time--;
    print_time();  // Display the remaining time for both players
}
void player2_timer(void* arg) {
    if (player2_time > 0) player2_time--;
    print_time();  // Display the remaining time for both players
}
void display_timer(void* arg) 
{
    // Sufficient size to hold concatenated minutes and seconds
    char min1[10];  // Increase the size of min1
    char sec1[10];  // Increase the size of sec1
    char min2[10];  // Increase the size of min2
    char sec2[10];  // Increase the size of sec2

    // Format minutes and seconds for Player 1
    sprintf(min1, "%02d", player1_time / (60 * 100));  // Minutes
    sprintf(sec1, "%02d", player1_time / 100 % 60);    // Seconds
    sprintf(min2, "%02d", player2_time / (60 * 100));  // Minutes
    sprintf(sec2, "%02d", player2_time / 100 % 60);    // Seconds

    // Concatenate the two strings
    char time_display1[20];  // Sufficient size to hold both parts
    strcpy(time_display1, min1);  // Copy min1 into time_display1
    strcat(time_display1, ":");  // Add a ":" between minutes and seconds
    strcat(time_display1, sec1);  // Add seconds to time_display1

    char time_display2[20];  // Sufficient size to hold both parts
    strcpy(time_display2, min2);  // Copy min2 into time_display2
    strcat(time_display2, ":");  // Add a ":" between minutes and seconds
    strcat(time_display2, sec2);  // Add seconds to time_display2

    // Display the formatted text
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text_x3(&dev, 0, time_display1, 5, false);  // Display Player 1's time
    ssd1306_display_text_x3(&dev, 4, time_display2, 5, false);  // Display Player 2's time  (The second display start at line 4 because at x3 size each character take 4 line for a display with 8 line)
}




// Timer initialization function
void init_timer() {
    esp_timer_create_args_t timer_args_player1 = {
        .callback = player1_timer,
        .arg = NULL,
        .name = "player1_timer"
    };

    esp_timer_create_args_t timer_args_player2 = {
        .callback = player2_timer,
        .arg = NULL,
        .name = "player2_timer"
    };

    esp_timer_create_args_t timer_args_display = {
        .callback = display_timer,
        .arg = NULL,
        .name = "display_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args_player1, &timer_handle_player1));
    ESP_ERROR_CHECK(esp_timer_create(&timer_args_player2, &timer_handle_player2));
    ESP_ERROR_CHECK(esp_timer_create(&timer_args_display, &timer_handle_display));
}

// Function to configure the buttons with interrupts
void IRAM_ATTR button_player1_isr(void* arg) {
    // Toggle between players on button press
    player1_turn = true;
    player2_turn = false;

    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}
void IRAM_ATTR button_player2_isr(void* arg) {
    // Toggle between players on button press
    player2_turn = true;
    player1_turn = false;

    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}
void IRAM_ATTR button_reset_isr(void* arg) {
    // All timers are stopped and reset (push a player's button to resume)
    player2_turn = false;
    player1_turn = false;
    player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    player2_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(xSemaphore, NULL);

}
void IRAM_ATTR button_pause_isr(void* arg) {
    // Toggle between players on button press
    player2_turn = false;
    player1_turn = false;
    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}

// Function to configure the buttons
void init_buttons() {
    //Install interrupt services
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INR_FLAG_DEFAULT));
    // Config button for player 1
    esp_rom_gpio_pad_select_gpio(BUTTON_PLAYER1_GPIO);
    ESP_ERROR_CHECK(gpio_set_direction(BUTTON_PLAYER1_GPIO, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(BUTTON_PLAYER1_GPIO, GPIO_INTR_POSEDGE));    //Positive edge
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PLAYER1_GPIO, button_player1_isr, NULL));
    // Config button for player 2
    esp_rom_gpio_pad_select_gpio(BUTTON_PLAYER2_GPIO);
    ESP_ERROR_CHECK(gpio_set_direction(BUTTON_PLAYER2_GPIO, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(BUTTON_PLAYER2_GPIO, GPIO_INTR_POSEDGE));    //Positive edge
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PLAYER2_GPIO, button_player2_isr, NULL));
    // Config button for reset
    esp_rom_gpio_pad_select_gpio(BUTTON_RESET_GPIO);
    ESP_ERROR_CHECK(gpio_set_direction(BUTTON_RESET_GPIO, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(BUTTON_RESET_GPIO, GPIO_INTR_POSEDGE));    //Positive edge
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_RESET_GPIO, button_reset_isr, NULL));
    // Config button for pause
    esp_rom_gpio_pad_select_gpio(BUTTON_PAUSE_GPIO);
    ESP_ERROR_CHECK(gpio_set_direction(BUTTON_PAUSE_GPIO, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(BUTTON_PAUSE_GPIO, GPIO_INTR_POSEDGE));    //Positive edge
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PAUSE_GPIO, button_pause_isr, NULL));
}

// Task to manage player 1's timer
void player1_task(void* arg) {
    while (1) {
        if (player1_turn) {
            // Set timer of player 1 as running
            player1_timer_running = true;
            // Start player 1's timer
            if (!esp_timer_is_active(timer_handle_player1)) {
                ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_player1, 10000));  // Start timer with an interval of 0.01 second (10000 µs)
            }
            // Suspend player 1 task until button press resumes it
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
        } else {
            if (player1_timer_running == true)
            {
                // Stop player 1's timer when it's not their turn
                ESP_ERROR_CHECK(esp_timer_stop(timer_handle_player1));
                player1_timer_running = false;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Task to manage player 2's timer
void player2_task(void* arg) {
    while (1) {
        if (player2_turn) {
            // Set timer of player 2 as running
            player2_timer_running = true;
            // Start player 2's timer
            if (!esp_timer_is_active(timer_handle_player2)) {
                ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_player2, 10000));  // Start timer with an interval of 0.01 second (10000 µs)
            }
            // Suspend player 2 task until button press resumes it
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
        } else {
            if (player2_timer_running == true)
            {
                // Stop player 2's timer when it's not their turn
                ESP_ERROR_CHECK(esp_timer_stop(timer_handle_player2));
                player2_timer_running = false;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}



void app_main() {

    

    // CONFIG_I2C_INTERFACE
    #if CONFIG_I2C_INTERFACE
        ESP_LOGI(tag, "INTERFACE is i2c");
        ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
        ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
        ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
        i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    #endif 
    // CONFIG_SPI_INTERFACE
    #if CONFIG_SPI_INTERFACE
        ESP_LOGI(tag, "INTERFACE is SPI");
        ESP_LOGI(tag, "CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
        ESP_LOGI(tag, "CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
        ESP_LOGI(tag, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
        ESP_LOGI(tag, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
        ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
        spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
    #endif 

    #if CONFIG_FLIP
	dev._flip = true;
	ESP_LOGW(tag, "Flip upside down");
    #endif
    // CONFIG_SSD1306_128x64
    #if CONFIG_SSD1306_128x64
        ESP_LOGI(tag, "Panel is 128x64");
        ssd1306_init(&dev, 128, 64);
    #endif 
    // CONFIG_SSD1306_128x32
    #if CONFIG_SSD1306_128x32
        ESP_LOGI(tag, "Panel is 128x32");
        ssd1306_init(&dev, 128, 32);
    #endif 

    // Initialize semaphore
    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
        ESP_LOGE("app_main", "Failed to create semaphore");
        return;
    }

    //Clear the screen once
    ssd1306_clear_screen(&dev, false);

    // Initialize buttons and timers
    init_buttons();
    init_timer();

    // Create tasks for each player
    xTaskCreate(player1_task, "player1_task", 2048, NULL, 5, NULL);
    xTaskCreate(player2_task, "player2_task", 2048, NULL, 5, NULL);

    //Start display timer (interval of 1 second)
    if (!esp_timer_is_active(timer_handle_display)) {
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_display, 1000000));  // Start timer with an interval of 1 second for displaying(1000000 µs)
    }
    // Display initial time
    print_time();
}
