#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "buttons.h"
#include "menu.h"

// GPIO for buttons
#define BUTTON_PLAYER1_GPIO 2
#define BUTTON_PLAYER2_GPIO 4
#define BUTTON_MINUS_GPIO 5
#define BUTTON_PLUS_GPIO 18
#define BUTTON_LEFT_GPIO 16
#define BUTTON_RIGHT_GPIO 17
#define BUTTON_MENU_GPIO 19

// Interruption
#define ESP_INR_FLAG_DEFAULT 0

//extern variables in main.c
extern int PLAYER_TIME;
extern int PLAYER_INC;
extern int player1_time;
extern int player2_time;  
extern int player1_inc; 
extern int player2_inc;  

//Turn flag
extern bool player1_turn;
extern bool player2_turn; 

extern bool player1_win;
extern bool player2_win;

// Semaphore to synchronize tasks
extern SemaphoreHandle_t clk_Semaphore;
//Queue
extern QueueHandle_t Menu_cmd_queue;
extern menu_state_t state;

// Function to configure the buttons with interrupts
void IRAM_ATTR button_player1_isr(void* arg) {
    //increment after a push
    if (player2_turn == true && player1_turn == false && player1_win == false && player2_win == false)  //if it was the player2's turn
    player2_time = player2_time + player2_inc;   //Increment
    // Toggle between players on button press
    player1_turn = true;
    player2_turn = false;
    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(clk_Semaphore, NULL);
}
void IRAM_ATTR button_player2_isr(void* arg) {
    //increment after a push
    if (player1_turn == true && player2_turn == false && player1_win == false && player2_win == false)  //if it was the player1's turn
        player1_time = player1_time + player1_inc;   //Increment
    // Toggle between players on button press
    player2_turn = true;
    player1_turn = false;
    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(clk_Semaphore, NULL);
}

void IRAM_ATTR button_minus_isr(void* arg) {
    input_event_t event_minus = INPUT_MINUS;
    xQueueSendFromISR(Menu_cmd_queue, &event_minus, (TickType_t) 0);
}

void IRAM_ATTR button_plus_isr(void* arg) {
    input_event_t event_plus = INPUT_PLUS;
    xQueueSendFromISR(Menu_cmd_queue, &event_plus, (TickType_t) 0);
}

void IRAM_ATTR button_left_isr(void* arg) {
    input_event_t event_left = INPUT_LEFT;
    xQueueSendFromISR(Menu_cmd_queue, &event_left, (TickType_t) 0);
}

void IRAM_ATTR button_right_isr(void* arg) {
    input_event_t event_right = INPUT_RIGHT;
    xQueueSendFromISR(Menu_cmd_queue, &event_right, (TickType_t) 0);
}

void IRAM_ATTR button_menu_isr(void* arg) { 
    input_event_t event_ok = INPUT_OK;
    xQueueSendFromISR(Menu_cmd_queue, &event_ok, (TickType_t) 0);
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
    gpio_setup_isr(BUTTON_MINUS_GPIO, button_minus_isr);
    // Config button for pause
    gpio_setup_isr(BUTTON_PLUS_GPIO, button_plus_isr);
    // Config button for minus
    gpio_setup_isr(BUTTON_LEFT_GPIO, button_left_isr);
    // Config button for plus
    gpio_setup_isr(BUTTON_RIGHT_GPIO, button_right_isr);
    // Config button for menu
    gpio_setup_isr(BUTTON_MENU_GPIO, button_menu_isr);
}



