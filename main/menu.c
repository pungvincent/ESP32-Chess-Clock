#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "menu.h"
#include "i2c-lcd.h"

//extern variables
extern unsigned int PLAYER_TIME;
extern unsigned int PLAYER_INC;
extern unsigned int player1_time;
extern unsigned int player2_time;  
extern unsigned int player1_inc; 
extern unsigned int player2_inc;  

//Turn flag
extern bool player1_turn;
extern bool player2_turn;  
// Semaphore to synchronize tasks
extern SemaphoreHandle_t clk_Semaphore;

const uint8_t menu_positions[][3] = {
    {0, 0, 1}, // MENU_SELECT_BLITZ
    {0, 7, 1}, // MENU_SELECT_BULLET
    {1, 0, 1}, // MENU_SELECT_RAPID
    {1, 7, 1}, // MENU_SELECT_CLASSICAL
    {0, 0, 2}, // MENU_SELECT_CUSTOM
    {0, 8, 2}, // MENU_SELECT_PAUSE
    {1, 0, 2}, // MENU_SELECT_RESET
    {1, 8, 2}  // MENU_SELECT_BACK
};

const uint8_t custom_positions[][3] = {
    {0, 0, 1}, // CUSTOM_SELECT_TIME
    {0, 7, 1}, // CUSTOM_SELECT_INC
    {0, 0, 2}, // CUSTOM_SELECT_CONFIRM
    {0, 9, 2}, // CUSTOM_SELECT_RESET
    {1, 0, 2} // CUSTOM_SELECT_BACK
};

const uint8_t custom_set_positions[][3] = {
    {0, 3, 1}, // CUSTOM_SET_SELECT_P1_MIN_TENTH
    {0, 4, 1}, // CUSTOM_SET_SELECT_P1_MIN_UNIT
    {0, 6, 1}, // CUSTOM_SET_SELECT_P1_SEC_TENTH
    {0, 7, 1}, // CUSTOM_SET_SELECT_P1_SEC_UNIT
    {0, 11, 1},// CUSTOM_SET_SELECT_P2_MIN_TENTH
    {0, 12, 1}, // CUSTOM_SET_SELECT_P2_MIN_UNIT
    {0, 14, 1}, // CUSTOM_SET_SELECT_P2_SEC_TENTH
    {0, 15, 1}, // CUSTOM_SET_SELECT_P2_SEC_UNIT
    {0, 0, 2}, // CUSTOM_SET_SELECT_INC
    {0, 8, 2}, // CUSTOM_SET_SELECT_CONFIRM
    {1, 8, 2} // CUSTOM_SET_SELECT_BACK
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

//Set the same timer for each player
void set_clk_settings(int p_time, int i_time) {
    // All timers are stopped and reset (push a player's button to resume)
    player2_turn = false;
    player1_turn = false;

    PLAYER_TIME= p_time;
    PLAYER_INC = i_time;
    player1_time = PLAYER_TIME*100;  //Convert to hundredth of a second
    player2_time = PLAYER_TIME*100; 
    player1_inc = PLAYER_INC*100;  
    player2_inc = PLAYER_INC*100;  
    // Give the semaphore to resume the corresponding task
    xSemaphoreGive(clk_Semaphore);
}

//Set different timer for each player
void set_custom_clk_settings(int p1_time, int p2_time, int i_time) {
    // All timers are stopped and reset (push a player's button to resume)
    player2_turn = false;
    player1_turn = false;

    player1_time = p1_time*100;  //Convert to hundredth of a second
    player2_time = p2_time*100; 
    player1_inc = i_time*100;  
    player2_inc = i_time*100;  
    // Give the semaphore to resume the corresponding task
    xSemaphoreGive(clk_Semaphore);
}

void display_menu_cursor(menu_options_t menu_options) {
    lcd_clear_player1 (); lcd_clear_player2 (); lcd_display_menu();
    if (menu_options >= MENU_SELECT_BLITZ && menu_options <= MENU_SELECT_BACK) {
        if (menu_positions[menu_options][2] == 1)
            lcd_put_cur_player1(menu_positions[menu_options][0], menu_positions[menu_options][1]);
        else
            lcd_put_cur_player2(menu_positions[menu_options][0], menu_positions[menu_options][1]);

        (menu_positions[menu_options][2] == 1 ? lcd_send_string_player1 : lcd_send_string_player2)(">");
    }
}

void display_custom_cursor(custom_options_t custom_options) {
    lcd_clear_player1 (); lcd_clear_player2 (); lcd_display_custom();
    if (custom_options >= CUSTOM_SELECT_TIME && custom_options <= CUSTOM_SELECT_BACK) {
        if (custom_positions[custom_options][2] == 1)
            lcd_put_cur_player1(custom_positions[custom_options][0], custom_positions[custom_options][1]);
        else
            lcd_put_cur_player2(custom_positions[custom_options][0], custom_positions[custom_options][1]);

        (custom_positions[custom_options][2] == 1 ? lcd_send_string_player1 : lcd_send_string_player2)(">");
    }
}

void display_custom_set_cursor(custom_set_options_t custom_set_options) {
    lcd_clear_player1 (); lcd_clear_player2 (); lcd_display_set_custom();
    if (custom_set_options >= CUSTOM_SET_SELECT_P1_MIN_TENTH && custom_set_options <= CUSTOM_SET_SELECT_BACK) {
        if (custom_set_positions[custom_set_options][2] == 1)
            lcd_put_cur_player1(custom_set_positions[custom_set_options][0], custom_set_positions[custom_set_options][1]);
        else
            lcd_put_cur_player2(custom_set_positions[custom_set_options][0], custom_set_positions[custom_set_options][1]);

        (custom_set_positions[custom_set_options][2] == 1 ? lcd_send_string_player1 : lcd_send_string_player2)(".");
    }
}