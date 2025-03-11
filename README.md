# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF.

# Version 2.0
The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each hundredth of seconds and each second on an LCD display 16x2. This version implements two buttons (one for each player), a pause button, and a reset button.

## Improvement
- Added a LCD Display

## Code

Further explication of the code can be found [here](Code.md).

## Future improvement

- add a second display for each player
- Make the digits bigger


# Resources
This project uses the following tutorial for driving the 16x2 LCD display:
- [How to use I2C in ESP32 || Interface LCD16x2](https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/#info_box)

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
This project uses the [Interface LCD16x2 Tutorial](https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/#info_box). Please refer to the repository for more details.