#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/i2c.h"

#include "buttons.h"
#include "i2c-lcd.h"

//i2c Configuration
static const char *TAG = "i2c-lcd";
#define I2C_MASTER_SCL_IO           GPIO_NUM_22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                      /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000


//i2c master initialization
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Initial time for each player (3 minutes = 180 seconds)
const int PLAYER_TIME = 180;
int player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
int player2_time = PLAYER_TIME*100;  //Convert to hundredth of a second

//Turn flag
bool player1_turn = false;
bool player2_turn = false;  

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

    //Display the result in the LCD Display;
    lcd_display_chess_counter(time_display1, time_display2);
    
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

    // Initialize semaphore
    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
        ESP_LOGE("app_main", "Failed to create semaphore");
        return;
    }

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    lcd_init();
    lcd_clear();

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
