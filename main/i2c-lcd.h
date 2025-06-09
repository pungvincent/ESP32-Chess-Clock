void lcd_init_player1 (void);   // initialize lcd 1
void lcd_init_player2 (void);   // initialize lcd 2

void lcd_send_cmd (char cmd, uint8_t SLAVE_ADDRESS_LCD);  // send command to the lcd
void lcd_send_data (char data, uint8_t SLAVE_ADDRESS_LCD);  // send data to the lcd

void lcd_send_string_player1 (char *str);  // send string to the lcd
void lcd_send_string_player2 (char *str);

void lcd_put_cur_player1(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);
void lcd_put_cur_player2(int row, int col);

void lcd_clear_player1 (void);
void lcd_clear_player2 (void);

//Display the counter for both displays
void lcd_display_chess_counter(char time_display1[], char time_display2[]);
void lcd_display_menu();
void lcd_display_custom();
void lcd_display_set_custom();
void lcd_display_custom_digit();
void lcd_display_custom_set_digit();

//Display Winner screen
void lcd_display_p1_winner();
void lcd_display_p2_winner();