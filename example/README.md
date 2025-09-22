# AHT20 Library Example
This example demonstrates how to use functions provided in the library to initialize
the AHT20 sensor and get temperature and humidity data. The example application prints 
temperature and relative humidity values to the console every 5 seconds.

# Wiring
Connect the pins on the sensor to the corresponding pins on the single board computer. 
Please see the [datasheet](https://files.seeedstudio.com/wiki/Grove-AHT20_I2C_Industrial_Grade_Temperature_and_Humidity_Sensor/AHT20-datasheet-2020-4-16.pdf) for more information.

| AHT20 Pin # | Pin Name  |
|-------------|-----------|
| 2           | VDD       |
| 3           | SCL       |
| 4           | SDA       |
| 5           | GND       |


# Build and Run Example
In **main.c**, update the **I2C_BUS** define to the correct I2C bus to use for your device.
Bus 1 is the default I2C bus for Raspberry Pi. Use the commands below to build and run the 
example application.

    gcc main.c -laht20 -llgpio -o aht20_app
    ./aht20_app

A sample of the example output is below.

    Temp: 21.67C 71.01F
    Relative Humidity: 60.76%

