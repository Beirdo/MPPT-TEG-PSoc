# I2C Map

Since I have a lot of I2C devices, both hooked up to the CPU and to the MCU, I figured it was a good time to document them

## CPU

| Address | Part Number       | Description |
| ------- | :---------------- | :---------- |
| ???? | ATECCx08 | Encryption helper chip on ESP32 board (can be programmed to any address) |
| 0x20 | TCA9534 | I/O Expander |
| 0x21 | TCA9534 | I/O Expander |
| 0x22 | TCA9534 | I/O Expander |
| 0x29 | LTR-303ALS | Ambient Light Sensor |
| 0x40 | INA219 | Voltage/Current/Power Measurement |
| 0x41 | INA219 | Voltage/Current/Power Measurement |
| 0x42 | INA219 | Voltage/Current/Power Measurement |
| 0x43 | INA219 | Voltage/Current/Power Measurement |
| 0x50 | MAX31760 | Fan Speed Controller |

## MCU

| Address | Part Number       | Description |
| ------- | :---------------- | :---------- |
| 0x20 | TCA9534 | I/O Expander (Board 0) |
| 0x21 | TCA9534 | I/O Expander (Board 1)|
| 0x70 | TCA9546 | I2C Bus Multiplexer (1:4) (Board 0) |
| 0x71 | TCA9546 | I2C Bus Multiplexer (1:4) (Board 1) |
| 0x40 | INA219 | Voltage/Current/Power Measurement (on each of the 8 multiplexed busses) |
| 0x41 | INA219 | Voltage/Current/Power Measurement (on each of the 8 multiplexed busses) |
| 0x42 | INA219 | Voltage/Current/Power Measurement (on each of the 8 multiplexed busses) |
| 0x48 | TMP100 | Temperature sensor (on each of the 8 multiplexed busses) |
| 0x49 | TMP100 | Temperature sensor (on each of the 8 multiplexed busses) |
