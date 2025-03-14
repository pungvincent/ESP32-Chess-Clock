# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF.

# Version 2.1
The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each hundredth of seconds and each second on 2 LCD screens 16x2 for each player. This version implements two buttons (one for each player), a pause button, and a reset button.

## Improvement
- Added a second LCD screen

## Code

Further explication of the code can be found [here](docs/Code.md).

## LCD 16x2 screen

Each LCD screen is driven by a PCF8574 module. To differentiate them, each LCD must have a unique I2C address.
Further explanation can be found [here](docs/LCD.md).

Here is the current configuration: 

| MSB  | A2 | A1 | A0 | I2C Address | Assigned To  |
|------|----|----|----|-------------|--------------|
| 0100 |  1 |  1 |  1 | 0x27        | Player 1 LCD |
| 0100 |  1 |  1 |  0 | 0x26        | Player 2 LCD |

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