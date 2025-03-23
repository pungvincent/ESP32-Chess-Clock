# Functions

|Name|Description|
|----|-----------|
|`void initialize_times()`|//Convert all seconds variables into hundredth of a second. |
|`void print_time()`|Displays the time remaining for each player on the serial monitor. |
|`void init_timer()`|Initializes each player's timers and sets their callback function. |
|`void player1_timer(void* arg)` and `void player2_timer(void* arg)`|Callback functions for each player's timer, which will be called every second to decrement the timer.|
|`void display_timer(void* arg)`|Callback function: Convert the "player_time" into minutes and second to display it on the lcd screen. |
|`void player1_task(void* arg)` and `void player2_task(void* arg)`| Manages the player timer, starting it when it's the player turn and suspending the task until a button press resumes it. If it's not the player turn, the timer is stopped, and the task waits briefly before checking again.|
|`void menu_task(void *arg) `|Manages the menu, starting when the `Menu/OK` button is pressed. Displays all presets on the LCD screen and manages the selected preset with the `+` and `-` button. |

## Buttons

|Name|Description|
|----|-----------|
|`void init_buttons()`|Sets all GPIO INPUT which will trigger an interruption routine.|
|`esp_err_t gpio_setup_isr(int gpio_num, gpio_isr_t isr_handler)`| Sets up an interrupt service routine (ISR) for a specified GPIO pin, configuring it as an input with a positive edge interrupt and assigning the ISR handler  |
|`void IRAM_ATTR button_player1_isr(void* arg)` and `void IRAM_ATTR button_player2_isr(void* arg)`|Interrupt triggered by the falling edge of `BUTTON_PLAYER_GPIO`(GPIO 2). It changes the player's turn with the `player1_turn` boolean/flag, and thus, the timer starts decrementing every second. It also gives back the `xSemaphore` binary semaphore at the same time.|
|`void IRAM_ATTR button_reset_isr(void* arg)`| Reset time interrupt. |
|`void IRAM_ATTR button_pause_isr(void* arg)`| Pause time interrupt. |
|`void IRAM_ATTR button_minus_isr(void* arg)`| Send `INPUT_DOWN` event to a queue which will be received by `menu_task`. |
|`void IRAM_ATTR button_plus_isr(void* arg)`| Send `INPUT_UP` event to a queue which will be received by `menu_task`. |
|`void IRAM_ATTR button_menu_isr(void* arg)`| Send `INPUT_OK` event to a queue which will be received by `menu_task`. |

## LCD

|Name|Description|
|----|-----------|
|`void lcd_init_player1 (void)`|Initializes Player 1’s LCD (4-bit mode, display settings, etc.).|
|`void lcd_init_player2 (void)`|Initializes Player 2’s LCD (same as for Player 1).|

`void lcd_send_cmd (char cmd, uint8_t SLAVE_ADDRESS_LCD)`: This function sends a command to the LCD via I2C. Commands are special instructions that control the LCD's behavior (e.g., clearing the screen, setting cursor position, enabling/disabling display, etc.).
- The command (cmd) is split into two parts: higher nibble (4 MSB bits) and lower nibble (4 LSB bits). This is necessary because the LCD operates in 4-bit mode, requiring two transmissions per command.
- The function formats the command bytes with control bits (EN for enable, RS for register select) to properly send the instruction.
- It then sends the data over I2C using i2c_master_write_to_device().
- If there’s an error during transmission, it logs an error message.

`void lcd_send_data (char data, uint8_t SLAVE_ADDRESS_LCD)`: This function sends character data to be displayed on the LCD. Unlike lcd_send_cmd, this function writes characters (letters, numbers, symbols) to the LCD's screen.
- The character (data) is also split into two parts (higher nibble and lower nibble) for 4-bit mode communication.
- It formats the data with control bits (RS = 1 to indicate data mode, and EN to trigger writing).
- The function then transmits the formatted data via I2C.
- If an error occurs during transmission, it logs an error message.

|Name|Description|
|----|-----------|
|`void lcd_send_string_player1 (char *str)`|Displays a string on Player 1's LCD.|
|`void lcd_send_string_player2 (char *str)`|Displays a string on Player 2's LCD.|
|`void lcd_put_cur_player1(int row, int col)`|Moves the cursor to a specific position on Player 1's LCD.|
|`void lcd_put_cur_player2(int row, int col)`|Moves the cursor to a specific position on Player 2's LCD.|
|`void lcd_clear_player1 (void)`|Clears the LCD screen for Player 1. |
|`void lcd_clear_player2 (void)`|Clears the LCD screen for Player 2. |
|`void lcd_display_chess_counter(char time_display1[], char time_display2[])`|Displays the chess timer values for both players on their respective LCDs. |
|`void lcd_display_menu()`| Displays all presets on the menu. |

# Menu

|Name|Description|
|----|-----------|
|`void pause_clk()`| Pause the timers for both players. |
|`void reset_clk()`| Reset the timers for both players. |
|`void set_clk_settings(int p_time, int i_time)`| Define the new Timer value and increment value in seconds. |
|`void display_menu_cursor(menu_options_t menu_options)`| Move the cursor next to the selected option/preset. |