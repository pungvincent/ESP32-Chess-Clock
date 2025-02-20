# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF

# Version 1.0
This first version consists of a generic chess clock. The timer is set up at 3 minutes (Blitz) and it displays the time remaining for each player on the serial monitor each second. The clock switch of turn at each falling edge of GPIO 2. 

## Code

Further explication of the code can be found [here](Code.md).

## Future Improvements

- This code is limited by the number of digits for seconds; it would be an improvement to add more digits after the decimal point.

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
