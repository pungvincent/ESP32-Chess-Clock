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
#include "freertos/queue.h"

#include "buttons.h"
#include "i2c-lcd.h"
#include "menu.h"

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

// Initial time for each player (3 minutes = 180 seconds and increment time = 2 seconds)
int PLAYER_TIME = 180;
int PLAYER_INC = 2;
int player1_time;        
int player2_time;       
int player1_inc;       
int player2_inc;   

//Turn flag
bool player1_turn = false;
bool player2_turn = false;  

// Semaphore to synchronize tasks
SemaphoreHandle_t clk_Semaphore = NULL;

// Timer variables
esp_timer_handle_t timer_handle_player1;
esp_timer_handle_t timer_handle_player2;
esp_timer_handle_t timer_handle_display;

//Track Variables
bool player1_timer_running = false;  // Variable to track if player 1's timer is running
bool player2_timer_running = false;  // Variable to track if player 2's timer is running

QueueHandle_t Menu_cmd_queue;
menu_options_t menu_options = MENU_SELECT_BLITZ;
menu_state_t menu_state = MENU_CLOSED;
input_event_t event;

void initialize_times() {
    player1_time = PLAYER_TIME * 100;  //Convert to hundredth of a second
    player2_time = PLAYER_TIME * 100; 
    player1_inc = PLAYER_INC * 100;    
    player2_inc = PLAYER_INC * 100;  
}

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
    if (menu_state == MENU_CLOSED) {
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
            xSemaphoreTake(clk_Semaphore, portMAX_DELAY);

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
            xSemaphoreTake(clk_Semaphore, portMAX_DELAY);
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



void menu_task(void *arg) {
    while (1) {
        if (xQueueReceive(Menu_cmd_queue, &(event), (TickType_t) 5)) {
            switch (event) {
                //Even if menu_options change when the menu isn't opened, it will be reseted to blitz(1rst option) when we'll open the menu
                case INPUT_DOWN:
                    if (menu_state == MENU_OPEN) {
                        if (menu_options > 0) {
                            menu_options--;  // Previous option
                        } else {
                            menu_options = MENU_SELECT_BACK;  // looping
                        }
                        //Move the cursor to the left
                        display_menu_cursor(menu_options);
                    }
                    break;
                //Even if menu_options change when the menu isn't opened, it will be reseted to blitz(1rst option) when we'll open the menu
                case INPUT_UP:
                    if (menu_state == MENU_OPEN) {
                        if (menu_options < MENU_SELECT_COUNT - 1) {
                            menu_options++;  // Next option

                        } else {
                            menu_options = MENU_SELECT_BLITZ;  // looping
                        }
                        //Move the cursor to the right
                        display_menu_cursor(menu_options);
                    }
                    break;
                // Two behaviors : if menu not open -> open menu button / if menu open -> ok button 
                case INPUT_OK:
                    switch (menu_state) {
                        case MENU_CLOSED:
                            pause_clk();
                            menu_state = MENU_OPEN; //If the menu is closed, then open it
                            menu_options = MENU_SELECT_BLITZ; //Set the cursor to the first option (Blitz)
                            //display menu and display the cursor at blitz
                            display_menu_cursor(menu_options);
                            printf("Menu opened\n");
                                //Display the menu
                            break;
                        case MENU_OPEN:
                            //Custom option
                            if (menu_options == MENU_SELECT_CUSTOM) {
                                printf("Custom selected\n");
                                // open new window here
                                return;
                            }
                            
                            //Preconfig options
                            switch (menu_options) {
                                case MENU_SELECT_BLITZ:      set_clk_settings(180, 2);   break;
                                case MENU_SELECT_BULLET:     set_clk_settings(60, 1);    break;
                                case MENU_SELECT_RAPID:      set_clk_settings(600, 0);   break;
                                case MENU_SELECT_CLASSICAL:  set_clk_settings(3600, 30); break;
                                case MENU_SELECT_BACK:       printf("Back selected\n");  break;
                                default: return;
                            }
                            printf("%s selected\n", 
                                menu_options == MENU_SELECT_BLITZ ? "Blitz" :
                                menu_options == MENU_SELECT_BULLET ? "Bullet" :
                                menu_options == MENU_SELECT_RAPID ? "Rapid" :
                                menu_options == MENU_SELECT_CLASSICAL ? "Classical" : "");
                                
                            if (menu_options != MENU_SELECT_CUSTOM) {
                                menu_state = MENU_CLOSED;
                                lcd_clear_player1 (); lcd_clear_player2 ();
                                printf("Returning to Clock\n");
                            }
                            break;  
                    }
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}



void app_main() {

    //Init times values
    initialize_times();

    // Initialize semaphore
    clk_Semaphore = xSemaphoreCreateBinary();
    if (clk_Semaphore == NULL) {
        ESP_LOGE("app_main", "Failed to create semaphore");
        return;
    }

    //Initialize queue
    Menu_cmd_queue = xQueueCreate(10, sizeof(input_event_t));
    if (Menu_cmd_queue == NULL) {
        printf("Failed to create the queue.\n");
    }

    //i2c init
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    //lcd init
    lcd_init_player1();
    lcd_clear_player1();
    lcd_init_player2();
    lcd_clear_player2();

    // Initialize buttons and timers
    init_buttons();
    init_timer();

    // Create tasks for each player
    xTaskCreate(player1_task, "player1_task", 4096, NULL, 5, NULL);
    xTaskCreate(player2_task, "player2_task", 4096, NULL, 5, NULL);
    xTaskCreatePinnedToCore(menu_task, "menu_task", 4096, NULL, 10, NULL, 1);

    //Start display timer (interval of 1 second)
    if (!esp_timer_is_active(timer_handle_display)) {
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_display, 1000000));  // Start timer with an interval of 1 second for displaying(1000000 µs)
    }
    // Display initial time
    print_time();

}
