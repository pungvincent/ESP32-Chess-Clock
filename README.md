# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF.

# Version 1.3
The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each hundredth of seconds and each second on an oled screen ssd1306. This version implements two buttons (one for each player), a pause button, and a reset button.

## Improvement
- Added a reset button
- Added a pause button
- Added other button to set the time of each player
- Added an another button and display for player 2

## menuconfig
 You have to set this config value with menuconfig
 CONFIG_INTERFACE

 for i2c
 CONFIG_MODEL
 CONFIG_SDA_GPIO
 CONFIG_SCL_GPIO
 CONFIG_RESET_GPIO

 for SPI
 CONFIG_CS_GPIO
 CONFIG_DC_GPIO
 CONFIG_RESET_GPIO

## Code

Further explication of the code can be found [here](Code.md).

## Future improvement

- Because the 128*64 OLED display is too small, it would be ideal to use another two 16*2 LCD displays to display the time of each player. The current OLED Display would still be used to config all the setting.
- For aesthetic purpose, the oled display could show an animation in the center to show, to show whose turn it is.


# Resources
This project uses the following library for driving the SSD1306 OLED display:
- [SSD1306/SH1106 Driver for esp-idf - nopnop2002](https://github.com/nopnop2002/esp-idf-ssd1306?tab=readme-ov-file)
- [Interface OLED with ESP32 using ESP-IDF](https://esp32tutorials.com/oled-esp32-esp-idf-tutorial/)

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
This project uses the [esp-idf-ssd1306 library](https://github.com/nopnop2002/esp-idf-ssd1306) under the [MIT License](https://github.com/nopnop2002/esp-idf-ssd1306/blob/main/LICENSE). Please refer to the repository for more details.