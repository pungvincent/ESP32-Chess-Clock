
/** Put this in the src folder **/
#include <stdint.h>

#include "i2c-lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "unistd.h"

#define SLAVE_ADDRESS_LCD_player1 0x4E>>1 //change this according to your setup
#define SLAVE_ADDRESS_LCD_player2 0x4C>>1 //Player_1: 0x27 and player2: 0x26

esp_err_t err;

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