#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "buttons.h"
#include "menu.h"

// GPIO for buttons
#define BUTTON_PLAYER1_GPIO 2
#define BUTTON_PLAYER2_GPIO 4
#define BUTTON_RESET_GPIO 16
#define BUTTON_PAUSE_GPIO 17
#define BUTTON_MINUS_GPIO 5
#define BUTTON_PLUS_GPIO 18
#define BUTTON_MENU_GPIO 19

// Interruption
#define ESP_INR_FLAG_DEFAULT 0

//external variables in main.c
extern const int PLAYER_TIME;
extern int player1_time;  
extern int player2_time; 
extern bool player1_turn;
extern bool player2_turn;
extern SemaphoreHandle_t clk_Semaphore;
extern SemaphoreHandle_t menu_Semaphore;
extern QueueHandle_t Menu_cmd_queue;
extern menu_state_t state;
extern input_event_t event;

// Function to configure the buttons with interrupts
void IRAM_ATTR button_player1_isr(void* arg) {
    // Toggle between players on button press
    player1_turn = true;
    player2_turn = false;

    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(clk_Semaphore, NULL);
}
void IRAM_ATTR button_player2_isr(void* arg) {
    // Toggle between players on button press
    player2_turn = true;
    player1_turn = false;

    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(clk_Semaphore, NULL);
}

void IRAM_ATTR button_reset_isr(void* arg) {
    // All timers are stopped and reset (push a player's button to resume)
    player2_turn = false;
    player1_turn = false;
    player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    player2_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(clk_Semaphore, NULL);

}
void IRAM_ATTR button_pause_isr(void* arg) {
    // Toggle between players on button press
    player2_turn = false;
    player1_turn = false;
    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(clk_Semaphore, NULL);
}

void IRAM_ATTR button_minus_isr(void* arg) {
    event = INPUT_DOWN;
    if (xQueueSendFromISR(Menu_cmd_queue, &event, NULL) != pdPASS) {
        printf("Failed to send event to queue\n");
    } else {
        printf("Event sent to queue\n");
    }
}

void IRAM_ATTR button_plus_isr(void* arg) {
    event = INPUT_UP;
    if (xQueueSendFromISR(Menu_cmd_queue, &event, NULL)!= pdPASS) {
        printf("Failed to send event to queue\n");
    } else {
        printf("Event sent to queue\n");
    }
}

void IRAM_ATTR button_menu_isr(void* arg) {
    event = INPUT_OK;
    if (xQueueSendFromISR(Menu_cmd_queue, &event, NULL) != pdPASS) {
        printf("Failed to send event to queue\n");
    } else {
        printf("Event sent to queue\n");
    }

}



esp_err_t gpio_setup_isr(int gpio_num, gpio_isr_t isr_handler)
{
    // Select the GPIO
    esp_rom_gpio_pad_select_gpio(gpio_num);
    // Set the GPIO direction as input
    ESP_ERROR_CHECK(gpio_set_direction(gpio_num, GPIO_MODE_INPUT));
    // Set the interrupt type (here, positive edge)
    ESP_ERROR_CHECK(gpio_set_intr_type(gpio_num, GPIO_INTR_POSEDGE));
    // Add the ISR handler for this GPIO
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpio_num, isr_handler, NULL));
    return ESP_OK;
}


// Function to configure the buttons
void init_buttons() {
    //Install interrupt services
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INR_FLAG_DEFAULT));

    // Config button for player 1
    gpio_setup_isr(BUTTON_PLAYER1_GPIO, button_player1_isr);
    // Config button for player 2
    gpio_setup_isr(BUTTON_PLAYER2_GPIO, button_player2_isr);
    // Config button for reset
    gpio_setup_isr(BUTTON_RESET_GPIO, button_reset_isr);
    // Config button for pause
    gpio_setup_isr(BUTTON_PAUSE_GPIO, button_pause_isr);
    // Config button for minus
    gpio_setup_isr(BUTTON_MINUS_GPIO, button_minus_isr);
    // Config button for plus
    gpio_setup_isr(BUTTON_PLUS_GPIO, button_plus_isr);
    // Config button for menu
    gpio_setup_isr(BUTTON_MENU_GPIO, button_menu_isr);
}



