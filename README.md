# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF

# Version 1.2
This version consists of a generic chess clock. The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each hundredth of seconds and each second on an oled screen ssd1306. The clock switch of turn at each falling edge of GPIO 2. 

## Improvement
An oled screen ssd1306 128x64 was added as a display

## Code

Further explication of the code can be found [here](Code.md).

# Official Documentation:

- [SSD1306/SH1106 Driver for esp-idf - nopnop2002](https://github.com/nopnop2002/esp-idf-ssd1306?tab=readme-ov-file)
- [Interface OLED with ESP32 using ESP-IDF](https://esp32tutorials.com/oled-esp32-esp-idf-tutorial/)

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
