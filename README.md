# ESP32-Chess-Clock
ESP32-based chess clock using ESP-IDF.

# Version 2.2

This project consists of a dual LCD screen displaying the time counter for each player. When the clock starts, it defaults to a 3-minute timer with an increment of 2 seconds per move.

Several buttons are available to control the clock:
|Reset|Pause|Player1|Player2|-|+|menu/ok|
|-----|-----|-------|-------|-|-|-------|

The menu offers different presets:
- BLITZ
- BULLET
- RAPID
- CLASSICAL
- CUSTOM
- BACK

<u>Note</u>: These presets are not strictly representative of standard time settings and increments, as they may vary depending on the players or their preferences.

## Future Improvement

- The custom preset has not been implemented yet. When selected, the system crashes and reboots.
- add an incremental rotary encoder
- add a buzzer

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

# Resources
This project uses the following tutorial for driving the 16x2 LCD display:
- [How to use I2C in ESP32 || Interface LCD16x2](https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/#info_box)

# License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

This project uses ESP-IDF, which is licensed under the Apache License 2.0. See the [ESP-IDF repository](https://github.com/espressif/esp-idf) for more details.
This project uses the [Interface LCD16x2 Tutorial](https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/#info_box). Please refer to the repository for more details.