#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"

// GPIO for buttons
#define BUTTON_PLAYER_GPIO 2

// Interruption
#define ESP_INR_FLAG_DEFAULT 0

// Initial time for each player (3 minutes = 180 seconds)
#define PLAYER_TIME 180
static int player1_time = PLAYER_TIME;
static int player2_time = PLAYER_TIME;
static bool player1_turn = true;  // Player 1 starts

// Semaphore to synchronize tasks
SemaphoreHandle_t xSemaphore = NULL;

// Timer variables
esp_timer_handle_t timer_handle;
esp_timer_handle_t timer_handle2;

//Track Variables
bool player1_timer_running = false;  // Variable to track if player 1's timer is running
bool player2_timer_running = false;  // Variable to track if player 2's timer is running


// Function to display the remaining time
void print_time() {
    printf("Player 1: %02d:%02d  Player 2: %02d:%02d\n", player1_time / 60, player1_time % 60, player2_time / 60, player2_time % 60);
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

    ESP_ERROR_CHECK(esp_timer_create(&timer_args_player1, &timer_handle));
    ESP_ERROR_CHECK(esp_timer_create(&timer_args_player2, &timer_handle2));
}

// Function to configure the buttons with interrupts
void IRAM_ATTR button_isr_handler(void* arg) {
    // Toggle between players on button press
    player1_turn = !player1_turn;

    // Give the semaphore to resume the corresponding task
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}

// Function to configure the buttons
void init_buttons() {
    esp_rom_gpio_pad_select_gpio(BUTTON_PLAYER_GPIO);
    ESP_ERROR_CHECK(gpio_set_direction(BUTTON_PLAYER_GPIO, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(BUTTON_PLAYER_GPIO, GPIO_INTR_NEGEDGE));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INR_FLAG_DEFAULT));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PLAYER_GPIO, button_isr_handler, NULL));
}

// Task to manage player 1's timer
void player1_task(void* arg) {
    while (1) {
        if (player1_turn) {
            // Set timer of player 1 as running
            player1_timer_running = true;
            // Start player 1's timer
            if (!esp_timer_is_active(timer_handle)) {
                ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, 1000000));  // Start timer with an interval of 1 second
            }
            // Suspend player 1 task until button press resumes it
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
        } else {
            if (player1_timer_running == true)
            {
                // Stop player 1's timer when it's not their turn
                ESP_ERROR_CHECK(esp_timer_stop(timer_handle));
                player1_timer_running = false;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Task to manage player 2's timer
void player2_task(void* arg) {
    while (1) {
        if (!player1_turn) {
            // Set timer of player 2 as running
            player2_timer_running = true;
            // Start player 2's timer
            if (!esp_timer_is_active(timer_handle2)) {
                ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle2, 1000000));  // Start timer with an interval of 1 second
            }
            // Suspend player 2 task until button press resumes it
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
        } else {
            if (player2_timer_running == true)
            {
                // Stop player 2's timer when it's not their turn
                ESP_ERROR_CHECK(esp_timer_stop(timer_handle2));
                player2_timer_running = false;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void app_main() {
    // Initialize semaphore
    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
        ESP_LOGE("app_main", "Failed to create semaphore");
        return;
    }

    // Initialize buttons and timers
    init_buttons();
    init_timer();

    // Create tasks for each player
    xTaskCreate(player1_task, "player1_task", 2048, NULL, 5, NULL);
    xTaskCreate(player2_task, "player2_task", 2048, NULL, 5, NULL);

    // Display initial time
    print_time();
}
