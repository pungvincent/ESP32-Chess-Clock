# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF.

# Version 2.1
The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each hundredth of seconds and each second on 2 LCD display 16x2 for each player. This version implements two buttons (one for each player), a pause button, and a reset button.

## Improvement
- Added a second LCD Display

## Code

Further explication of the code can be found [here](Code.md).

## LCD 16x2 Display

When using two LCD screens on the same I2C bus, each screen must have a unique I2C address. By default, most I2C LCD modules use the address 0x27. To change the address of one of the screens, you need to modify the settings using the A0, A1, and A2 pins on the I2C module. I use two PCF8574 I2C module for my LCD screen.

These pins determine the three least significant bits (LSB) of the I2C address. The four most significant bits (MSB) are fixed at 0100 for this type of module. By default, A0, A1, and A2 are set to 1 (high). To change an address bit to 0 (low), you need to solder the corresponding pin to ground.

In my setup, I set the A0 pin to 0, which changed the I2C address from 0x27 to 0x26. The address follows this logic:
| MSB  | A2 | A1 | A0 | I2C Address |
|------|----|----|----|------------|
| 0100 |  1 |  1 |  1 | 0x27       |
| 0100 |  1 |  1 |  0 | 0x26       |
| 0100 |  1 |  0 |  1 | 0x25       |
| 0100 |  1 |  0 |  0 | 0x24       |
| 0100 |  0 |  1 |  1 | 0x23       |
| 0100 |  0 |  1 |  0 | 0x22       |
| 0100 |  0 |  0 |  1 | 0x21       |
| 0100 |  0 |  0 |  0 | 0x20       |
By configuring the I2C addresses correctly, both LCD screens can be used simultaneously without conflicts.

Here is the current config: 

| MSB  | A2 | A1 | A0 | I2C Address | Assigned To  |
|------|----|----|----|------------|-------------|
| 0100 |  1 |  1 |  1 | 0x27       | Player 1 LCD |
| 0100 |  1 |  1 |  0 | 0x26       | Player 2 LCD |

## Future improvement

- change the timer count
- add an incremental rotary encoder
- make a menu 
- add a buzzer



# Resources
This project uses the following tutorial for driving the 16x2 LCD display:
- [How to use I2C in ESP32 || Interface LCD16x2](https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/#info_box)

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
This project uses the [Interface LCD16x2 Tutorial](https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/#info_box). Please refer to the repository for more details.