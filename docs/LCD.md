# I2C address

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