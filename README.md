# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF

# Version 1.1
This version consists of a generic chess clock. The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each hundredth of seconds. The clock switch of turn at each falling edge of GPIO 2. 

## Improvement
The clock's precision was improved by switching from seconds to hundredths of a second.

## Code

Further explication of the code can be found [here](Code.md).

## Future Improvements

- An OLED display can be added to display the timer of each player

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
