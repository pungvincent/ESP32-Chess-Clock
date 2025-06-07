
/** Put this in the src folder **/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "i2c-lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "unistd.h"

#include "menu.h"

#define SLAVE_ADDRESS_LCD_player1 0x4E>>1 //change this according to your setup
#define SLAVE_ADDRESS_LCD_player2 0x4C>>1 //Player_1: 0x27 and player2: 0x26

esp_err_t err;

//Custom mode time and increment time
extern unsigned int custom_timer;   //in minutes
extern unsigned int custom_increment; // in secondes

//Custom set mode P1 and P2 time
extern unsigned int custom_set_p1_time;
extern unsigned int custom_set_p2_time;
extern unsigned int custom_set_inc_time;

#define I2C_NUM I2C_NUM_0

static const char *TAG = "LCD";

void lcd_send_cmd (char cmd, uint8_t SLAVE_ADDRESS_LCD)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
	if (err!=0) ESP_LOGI(TAG, "Error in sending command");
}

void lcd_send_data (char data, uint8_t SLAVE_ADDRESS_LCD)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
	if (err!=0) ESP_LOGI(TAG, "Error in sending data");
}

void lcd_clear_player1 (void)
{
	lcd_send_cmd (0x01, SLAVE_ADDRESS_LCD_player1);
	usleep(5000);
}

void lcd_clear_player2 (void)
{
	lcd_send_cmd (0x01, SLAVE_ADDRESS_LCD_player2);
	usleep(5000);
}


// put cursor at the entered position row (0 or 1), col (0-15);
void lcd_put_cur_player1(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col, SLAVE_ADDRESS_LCD_player1);
}

void lcd_put_cur_player2(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col, SLAVE_ADDRESS_LCD_player2);
}


void lcd_init_player1 (void)
{
	// 4 bit initialisation
	usleep(50000);  // wait for >40ms
	lcd_send_cmd (0x30, SLAVE_ADDRESS_LCD_player1);
	usleep(5000);  // wait for >4.1ms
	lcd_send_cmd (0x30, SLAVE_ADDRESS_LCD_player1);
	usleep(200);  // wait for >100us
	lcd_send_cmd (0x30, SLAVE_ADDRESS_LCD_player1);
	usleep(10000);
	lcd_send_cmd (0x20, SLAVE_ADDRESS_LCD_player1);  // 4bit mode
	usleep(10000);

  // dislay initialisation
	lcd_send_cmd (0x28, SLAVE_ADDRESS_LCD_player1); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	usleep(1000);
	lcd_send_cmd (0x08, SLAVE_ADDRESS_LCD_player1); //Display on/off control --> D=0,C=0, B=0  ---> display off
	usleep(1000);
	lcd_send_cmd (0x01, SLAVE_ADDRESS_LCD_player1);  // clear display
	usleep(1000);
	usleep(1000);
	lcd_send_cmd (0x06, SLAVE_ADDRESS_LCD_player1); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	usleep(1000);
	lcd_send_cmd (0x0C, SLAVE_ADDRESS_LCD_player1); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	usleep(1000);
}

void lcd_init_player2 (void)
{
	// 4 bit initialisation
	usleep(50000);  // wait for >40ms
	lcd_send_cmd (0x30, SLAVE_ADDRESS_LCD_player2);
	usleep(5000);  // wait for >4.1ms
	lcd_send_cmd (0x30, SLAVE_ADDRESS_LCD_player2);
	usleep(200);  // wait for >100us
	lcd_send_cmd (0x30, SLAVE_ADDRESS_LCD_player2);
	usleep(10000);
	lcd_send_cmd (0x20, SLAVE_ADDRESS_LCD_player2);  // 4bit mode
	usleep(10000);

  // dislay initialisation
	lcd_send_cmd (0x28, SLAVE_ADDRESS_LCD_player2); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	usleep(1000);
	lcd_send_cmd (0x08, SLAVE_ADDRESS_LCD_player2); //Display on/off control --> D=0,C=0, B=0  ---> display off
	usleep(1000);
	lcd_send_cmd (0x01, SLAVE_ADDRESS_LCD_player2);  // clear display
	usleep(1000);
	usleep(1000);
	lcd_send_cmd (0x06, SLAVE_ADDRESS_LCD_player2); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	usleep(1000);
	lcd_send_cmd (0x0C, SLAVE_ADDRESS_LCD_player2); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	usleep(1000);
}


void lcd_send_string_player1 (char *str)
{
	while (*str) lcd_send_data (*str++, SLAVE_ADDRESS_LCD_player1);
}

void lcd_send_string_player2 (char *str)
{
	while (*str) lcd_send_data (*str++, SLAVE_ADDRESS_LCD_player2);
}

//Display the counter for both displays
void lcd_display_chess_counter(char time_display1[], char time_display2[]) {
	//Display the counter on player's 1 display
	lcd_put_cur_player1(0, 0);
	lcd_send_string_player1("Player 1 time:");
	lcd_put_cur_player1(1, 5);
	lcd_send_string_player1(time_display1);
	//Display the counter on player's 2 display
	lcd_put_cur_player2(0, 0);
	lcd_send_string_player2("Player 2 time:");
	lcd_put_cur_player2(1, 5);
	lcd_send_string_player2(time_display2);
}

//Display the menu
void lcd_display_menu() {
	//Display the counter on player's 1 display
	lcd_put_cur_player1(0, 1);
	lcd_send_string_player1("Blitz");
	lcd_put_cur_player1(0, 8);
	lcd_send_string_player1("Bullet");
	lcd_put_cur_player1(1, 1);
	lcd_send_string_player1("Rapid");
	lcd_put_cur_player1(1, 8);
	lcd_send_string_player1("Classic");
	//Display the counter on player's 2 display
	lcd_put_cur_player2(0, 1);
	lcd_send_string_player2("Custom");
	lcd_put_cur_player2(0, 9);
	lcd_send_string_player2("Lichess");
	lcd_put_cur_player2(1, 1);
	lcd_send_string_player2("Reset");
	lcd_put_cur_player2(1, 9);
	lcd_send_string_player2("Back");
}

// Display the menu for custom mode
void lcd_display_custom() {
	//Display the counter on player's 1 display
	lcd_put_cur_player1(0, 1);
	lcd_send_string_player1("Timer");
	lcd_put_cur_player1(0, 8);
	lcd_send_string_player1("Inc");
	lcd_display_custom_digit();
	//Display the counter on player's 2 display
	lcd_put_cur_player2(0, 1);
	lcd_send_string_player2("Confirm");
	lcd_put_cur_player2(0, 10);
	lcd_send_string_player2("Set");
	lcd_put_cur_player2(1, 1);
	lcd_send_string_player2("Back");
}

void lcd_display_set_custom() {
	//Display the counter on player's 1 display
	lcd_put_cur_player1(0, 0);
	lcd_send_string_player1("P1:");
	lcd_put_cur_player1(0, 8);
	lcd_send_string_player1("P2:");
	//Display the counter on player's 2 display
	lcd_put_cur_player2(0, 1);
	lcd_send_string_player2("Inc:");
	lcd_display_custom_set_digit();
	lcd_put_cur_player2(0, 9);
	lcd_send_string_player2("Confirm");
	lcd_put_cur_player2(1, 9);
	lcd_send_string_player2("Back");
}

// Display the updated digits in custom mode
void lcd_display_custom_digit() {
	printf("%d" , custom_timer );
	// Buffers for formatted time
	char custom_timer_min[10];
	char custom_inc_sec[10];

	// Format minutes and seconds
	sprintf(custom_timer_min, "%02d", custom_timer);  // Minutes
	sprintf(custom_inc_sec, "%02d", custom_increment);  // Seconds

	// Build timer display (e.g., "05:00")
	char Custom_timer_display[20];
	strcpy(Custom_timer_display, custom_timer_min);
	strcat(Custom_timer_display, ":00");  // No need to change the sec


	// Build increment display (e.g., "00:30")
	char Custom_increment_display[20];
	strcpy(Custom_increment_display, "00:");
	strcat(Custom_increment_display, custom_inc_sec);

	// Display on LCD
	lcd_put_cur_player1(1, 1);
	lcd_send_string_player1(Custom_timer_display);
	lcd_put_cur_player1(1, 8);
	lcd_send_string_player1(Custom_increment_display);
}

// Display the updated digits in custom set mode
void lcd_display_custom_set_digit() {
	printf("%d" , custom_timer );
	// Buffers for formatted time
	char p1_min[10];
	char p1_sec[10];
	char p2_min[10];
	char p2_sec[10];
	char inc_min[10];
	char inc_sec[10];

	// Format minutes and seconds
	sprintf(p1_min, "%02d", custom_set_p1_time/60);  // Minutes
	sprintf(p1_sec, "%02d", custom_set_p1_time%60);  // Seconds
	sprintf(p2_min, "%02d", custom_set_p2_time/60);  // Minutes
	sprintf(p2_sec, "%02d", custom_set_p2_time%60);  // Seconds
	sprintf(inc_min, "%02d", custom_set_inc_time/60);  // Minutes
	sprintf(inc_sec, "%02d", custom_set_inc_time%60);  // Seconds

	// Build P1 display (e.g., "05:00")
	char Custom_set_p1_timer_display[20];
	strcpy(Custom_set_p1_timer_display, p1_min);
	strcat(Custom_set_p1_timer_display, ":");
	strcat(Custom_set_p1_timer_display, p1_sec); 

	// Build P2 display (e.g., "05:00")
	char Custom_set_p2_timer_display[20];
	strcpy(Custom_set_p2_timer_display, p2_min);
	strcat(Custom_set_p2_timer_display, ":");
	strcat(Custom_set_p2_timer_display, p2_sec); 

	// Build increment time display (e.g., "05:00")
	char Custom_set_inc_timer_display[20];
	strcpy(Custom_set_inc_timer_display, inc_min);
	strcat(Custom_set_inc_timer_display, ":"); 
	strcat(Custom_set_inc_timer_display, inc_sec); 

	// Display on LCD
	lcd_put_cur_player1(1, 3);
	lcd_send_string_player1(Custom_set_p1_timer_display);
	lcd_put_cur_player1(1, 11);
	lcd_send_string_player1(Custom_set_p2_timer_display);
	lcd_put_cur_player2(1, 1);
	lcd_send_string_player2(Custom_set_inc_timer_display);
}