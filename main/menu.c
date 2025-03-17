#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "menu.h"

//extern variables
extern const int PLAYER_TIME;
extern const int PLAYER_INC;
extern int player1_time;
extern int player2_time;  
extern int player1_inc; 
extern int player2_inc;  
//Turn flag
extern bool player1_turn;
extern bool player2_turn;  
// Semaphore to synchronize tasks
extern SemaphoreHandle_t clk_Semaphore;
extern SemaphoreHandle_t menu_Semaphore;

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

