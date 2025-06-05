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

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"

#include "buttons.h"
#include "i2c-lcd.h"
#include "menu.h"
#include "lichess.h"

//Wifi config
static const char *TAG_wifi = "WIFI";  // Logging tag for ESP_LOG functions
bool wifi_connected = false;  // Boolean to track connection status
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASS

//i2c Configuration
static const char *TAG = "i2c-lcd";
#define I2C_MASTER_SCL_IO           GPIO_NUM_22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                      /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

// Wi-Fi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG_wifi, "Disconnected. Reconnecting...");
        wifi_connected = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_wifi, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
    }
}

// Initialize Wi-Fi in station (STA) mode
void wifi_init_sta(void) {
    esp_netif_init();                         // Initialize TCP/IP stack
    esp_event_loop_create_default();          // Create default event loop
    esp_netif_create_default_wifi_sta();      // Create default Wi-Fi STA network interface

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  // Default Wi-Fi config
    esp_wifi_init(&cfg);                                  // Initialize Wi-Fi driver
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Set up Wi-Fi credentials
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);                    // Set to station mode
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);      // Apply config
    esp_wifi_start();                                    // Start Wi-Fi

    ESP_LOGI(TAG_wifi, "Wi-Fi Connexion...");
    esp_wifi_connect();                                  // Try to connect
}

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
unsigned int PLAYER_TIME = 180;
unsigned int PLAYER_INC = 2;
unsigned int player1_time;        
unsigned int player2_time;       
unsigned int player1_inc;       
unsigned int player2_inc;   

//Custom mode time and increment time
unsigned int custom_timer;   //in minutes
unsigned int custom_increment; // in secondes

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
custom_options_t custom_options = CUSTOM_SELECT_TIME;
menu_state_t menu_state = MENU_CLOSED;
custom_state_t custom_state = CUSTOM_CLOSED;
Lichess_state_t Lichess_state = LICHESS_CLOSED;
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
    if (menu_state == MENU_CLOSED && custom_state == CUSTOM_CLOSED && Lichess_state == LICHESS_CLOSED) {
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
                //Buttons for Custom mode
                case INPUT_MINUS:
                    if (custom_state == CUSTOM_OPEN) {
                        if (custom_options == CUSTOM_SELECT_TIME) {
                            if (custom_timer > 0) 
                            custom_timer--;
                        } 
                        else if (custom_options == CUSTOM_SELECT_INC) {
                            if (custom_increment > 0) 
                            custom_increment--;
                        }
                        lcd_display_custom_digit(custom_timer, custom_increment);
                    }
                    break;
                case INPUT_PLUS:
                    if (custom_state == CUSTOM_OPEN) {
                        if (custom_options == CUSTOM_SELECT_TIME) {
                            if (custom_timer < 100) 
                            custom_timer++;
                        } 
                        else if (custom_options == CUSTOM_SELECT_INC) {
                            if (custom_increment < 60) 
                            custom_increment++;
                        }
                        lcd_display_custom_digit(custom_timer, custom_increment);
                    }
                    break;
                case INPUT_LEFT:
                    if (menu_state == MENU_OPEN) {
                        if (menu_options > 0) {
                            menu_options--;  // Previous option
                        } else {
                            menu_options = MENU_SELECT_BACK;  // looping
                        }
                        //Move the cursor to the left option
                        display_menu_cursor(menu_options);
                    }
                    else if (custom_state == CUSTOM_OPEN) {
                        if (custom_options > 0) {
                            custom_options--;  // Previous option
                        } else {
                            custom_options = CUSTOM_SELECT_TIME;  // looping
                        }
                        //Move the cursor to the left option
                        display_custom_cursor(custom_options);
                    }
                    break;
                case INPUT_RIGHT:
                    if (menu_state == MENU_OPEN) {
                        if (menu_options < MENU_SELECT_COUNT - 1) {
                            menu_options++;  // Next option

                        } else {
                            menu_options = MENU_SELECT_BLITZ;  // looping
                        }
                        //Move the cursor to the right option
                        display_menu_cursor(menu_options);
                    }
                    else if (custom_state == CUSTOM_OPEN) {
                        if (custom_options < CUSTOM_SELECT_COUNT - 1) {
                            custom_options++;  // Next option

                        } else {
                            custom_options = CUSTOM_SELECT_TIME;  // looping
                        }
                        //Move the cursor to the right option
                        display_custom_cursor(custom_options);
                    }
                    break;
                // Two behaviors : if menu not open -> open menu button / if menu open -> ok button 
                case INPUT_OK:
                    if (custom_state == CUSTOM_OPEN)
                    {
                        switch(custom_options)
                        {
                            case CUSTOM_SELECT_CONFIRM:
                                set_clk_settings(custom_timer*60, custom_increment);
                                break;
                            case CUSTOM_SELECT_RESET:
                                break;
                            case CUSTOM_SELECT_BACK:
                                printf("Back selected\n");
                                break;
                            default:
                                break;
                        }
                        if (custom_options == CUSTOM_SELECT_CONFIRM || custom_options == CUSTOM_SELECT_BACK) {
                            custom_state = CUSTOM_CLOSED;
                            lcd_clear_player1 (); lcd_clear_player2 ();
                            menu_state = MENU_CLOSED;
                            printf("Returning to Clock\n");
                        }
                    }
                    else if (Lichess_state == LICHESS_OPEN) {
                        deactivate_lichess_mode(); // End of Lichess mode
                        lcd_clear_player1(); lcd_clear_player2();
                        lcd_put_cur_player1(0, 0); lcd_send_string_player1("Back to");
                        lcd_put_cur_player1(1, 0); lcd_send_string_player1("Local clock...");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        //Back to menu
                        menu_state = MENU_OPEN; Lichess_state = LICHESS_CLOSED;//return to menu
                        menu_options = MENU_SELECT_LICHESS; //Set back the cursor to lichess
                        display_menu_cursor(menu_options); //display menu and display the cursor at lichess
                        printf("Menu opened\n"); //Display the menu
                    }
                    else 
                    {
                        switch (menu_state) {
                            case MENU_CLOSED:
                                menu_state = MENU_OPEN; //If the menu is closed, then open it
                                menu_options = MENU_SELECT_BLITZ; //Set the cursor to the first option (Blitz)
                                display_menu_cursor(menu_options); //display menu and display the cursor at blitz
                                printf("Menu opened\n"); //Display the menu
                                break;

                            case MENU_OPEN:
                                //Stop the timer before we change the mode
                                pause_clk();
                                //Custom option
                                if (menu_options == MENU_SELECT_CUSTOM) {
                                    menu_state = MENU_CLOSED; custom_state = CUSTOM_OPEN;
                                    custom_timer = 0; custom_increment = 0; //Set the settings to 0
                                    custom_options = CUSTOM_SELECT_TIME;
                                    display_custom_cursor(custom_options); 
                                    printf("Custom selected\n");
                                }
                                //Lichess mode
                                else if (menu_options == MENU_SELECT_LICHESS) {
                                    menu_state = MENU_CLOSED; Lichess_state = LICHESS_OPEN;
                                    lcd_clear_player1(); lcd_clear_player2();
                                    lcd_put_cur_player1(0, 0); lcd_send_string_player1("Connection to");
                                    lcd_put_cur_player1(1, 0); lcd_send_string_player1("Wifi...");
                                    printf("Lichess mode opened\n");
                                    vTaskDelay(pdMS_TO_TICKS(3000)); 
                                    if (wifi_connected) {
                                        lcd_put_cur_player2(0, 3); lcd_send_string_player2("Connected!");
                                        vTaskDelay(pdMS_TO_TICKS(2000));
                                        activate_lichess_mode();
                                    } else {
                                        lcd_put_cur_player2(0, 1); lcd_send_string_player2("Not connected!");
                                        lcd_put_cur_player2(1, 0); lcd_send_string_player2(">Back to menu...");
                                        vTaskDelay(pdMS_TO_TICKS(2000)); 
                                        //Back to menu
                                        menu_state = MENU_OPEN; Lichess_state = LICHESS_CLOSED;//return to menu
                                        menu_options = MENU_SELECT_LICHESS; //Set back the cursor to lichess
                                        display_menu_cursor(menu_options); //display menu and display the cursor at lichess
                                        printf("Menu opened\n"); //Display the menu
                                        break;
                                    }
                                    lcd_clear_player1(); lcd_clear_player2();
                                }
                                else {
                                    //Preconfig options
                                    switch (menu_options) {
                                        case MENU_SELECT_BLITZ:      set_clk_settings(180, 2);   break;
                                        case MENU_SELECT_BULLET:     set_clk_settings(60, 1);    break;
                                        case MENU_SELECT_RAPID:      set_clk_settings(600, 0);   break;
                                        case MENU_SELECT_CLASSICAL:  set_clk_settings(3600, 30); break;
                                        case MENU_SELECT_RESET:      reset_clk();                break;
                                        case MENU_SELECT_BACK:       printf("Back selected\n");  break;
                                        default: return;
                                    }
                                    printf("%s selected\n", 
                                        menu_options == MENU_SELECT_BLITZ ? "Blitz" :
                                        menu_options == MENU_SELECT_BULLET ? "Bullet" :
                                        menu_options == MENU_SELECT_RAPID ? "Rapid" :
                                        menu_options == MENU_SELECT_CLASSICAL ? "Classical" : "");
                                    lcd_clear_player1 (); lcd_clear_player2 ();
                                    menu_state = MENU_CLOSED;
                                    printf("Returning to Clock\n");
                                }
                                break;  
                        }
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
    //i2c LCD init
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

    //Wifi
    lcd_put_cur_player1(0, 2); lcd_send_string_player1("Chess clock");
    lcd_put_cur_player2(0, 3); lcd_send_string_player2("Loading...");
    ESP_ERROR_CHECK(nvs_flash_init());      // Initialize NVS (required by Wi-Fi)
    wifi_init_sta();          // Connect to Wi-Fi
    vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for Wi-Fi to establish connection
    lcd_clear_player1(); lcd_clear_player2();

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

    // Create tasks for each player
    xTaskCreatePinnedToCore(player1_task, "player1_task", 4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(player2_task, "player2_task", 4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(menu_task, "menu_task", 4096, NULL, 4, NULL, 1);

    //Start display timer (interval of 1 second)
    if (!esp_timer_is_active(timer_handle_display)) {
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_display, 1000000));  // Start timer with an interval of 1 second for displaying(1000000 µs)
    }

    // Display initial time
    print_time();

}
