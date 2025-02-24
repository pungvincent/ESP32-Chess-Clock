#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "buttons.h"

// GPIO for buttons
#define BUTTON_PLAYER1_GPIO 2
#define BUTTON_PLAYER2_GPIO 4
#define BUTTON_RESET_GPIO 16
#define BUTTON_PAUSE_GPIO 17

// Interruption
#define ESP_INR_FLAG_DEFAULT 0

//external variables in main.c
extern const int PLAYER_TIME;
extern int player1_time;  
extern int player2_time; 
extern bool player1_turn;
extern bool player2_turn;
extern SemaphoreHandle_t xSemaphore;

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