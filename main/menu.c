#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "menu.h"
#include "i2c-lcd.h"

//extern variables
extern int PLAYER_TIME;
extern int PLAYER_INC;
extern int player1_time;
extern int player2_time;  
extern int player1_inc; 
extern int player2_inc;  

//Turn flag
extern bool player1_turn;
extern bool player2_turn;  
// Semaphore to synchronize tasks
extern SemaphoreHandle_t clk_Semaphore;

const uint8_t positions[][3] = {
    {0, 0, 1}, // MENU_SELECT_BLITZ
    {0, 7, 1}, // MENU_SELECT_BULLET
    {1, 0, 1}, // MENU_SELECT_RAPID
    {1, 7, 1}, // MENU_SELECT_CLASSICAL
    {0, 0, 2}, // MENU_SELECT_CUSTOM
    {0, 8, 2}  // MENU_SELECT_BACK
};

void pause_clk() {
    // Toggle between players on button press
    player2_turn = false;
    player1_turn = false;
    // Give the semaphore to resume the corresponding task
    xSemaphoreGive(clk_Semaphore);
}

void reset_clk() {
    // All timers are stopped and reset (push a player's button to resume)
    player2_turn = false;
    player1_turn = false;
    player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    player2_time = PLAYER_TIME*100; 
    player1_inc = PLAYER_INC*100;  
    player2_inc = PLAYER_INC*100;  
    // Give the semaphore to resume the corresponding task
    xSemaphoreGive(clk_Semaphore);
}

void set_clk_settings(int p_time, int i_time) {
    PLAYER_TIME= p_time;
    PLAYER_INC = i_time;
    player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    player2_time = PLAYER_TIME*100; 
    player1_inc = PLAYER_INC*100;  
    player2_inc = PLAYER_INC*100;  
}

void display_menu_cursor(menu_options_t menu_options) {
    lcd_clear_player1 (); lcd_clear_player2 (); lcd_display_menu();
    if (menu_options >= MENU_SELECT_BLITZ && menu_options <= MENU_SELECT_BACK) {
        if (positions[menu_options][2] == 1)
            lcd_put_cur_player1(positions[menu_options][0], positions[menu_options][1]);
        else
            lcd_put_cur_player2(positions[menu_options][0], positions[menu_options][1]);

        (positions[menu_options][2] == 1 ? lcd_send_string_player1 : lcd_send_string_player2)(">");
    }
}