/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/

#ifndef AHT20_H
#define AHT20_H

/* Includes */
#include <stdlib.h>
#include <unistd.h>
#include <lgpio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

/* Defines */

// Amount of bytes read from sensor when getting a measurement
#define SENSOR_DATA_LEN 7

// Conversion of Celcius to Fahrenheit
#define C_TO_F(x)  ((x * 9/5) + 32)

struct aht20_sensor
{
    char  buf[SENSOR_DATA_LEN];
    float humidity;
    int   temperature;
    int   lg_I2C_handle;
    int   I2C_dev;
};


/* Checks if data was correctly received by comparing the CRC8 byte received from
*  the sensor to the CRC8 that is calculated from the state byte and temperature/humidity
*  bytes received from the sensor.
*
*  in sensor - pointer to aht20_sensor struct used to provide CRC and data bytes
               received from the sensor
*  out - 1 (true) if the calculated CRC8 is the same as the CRC8 byte that was recieved from the sensor
*        0 (false) if the calculated CRC8 is different from the CRC8 byte that was recieved from the sensor
*/
bool crc_check(struct aht20_sensor *sensor);

/* Initializes the AHT20 sensor. Checks calibration bit and sends initialize command 
*  if the bit is 0, followed by a 10ms wait.
*
*  in sensor - pointer to aht20_sensor struct to initialize
*  out - None
*/
void aht20_init(struct aht20_sensor *sensor);

/* Gets a measurement from the sensor and performs calculations for temperature (in Celcuis)
*  and relative humidity percentage. Checks CRC of data received.
*
*  in sensor - pointer to aht20_sensor struct
*  out - 1 (true) if the calculated CRC8 is the same as the CRC8 byte that was recieved from the sensor
*        0 (false) if the calculated CRC8 is different from the CRC8 byte that was recieved from the sensor
*/
bool aht20_get_all_data(struct aht20_sensor *sensor);

/* Gets a measurement from the sensor and performs calculations for relative humidity
*  percentage. Checks CRC of data received.
*
*  in sensor - pointer to aht20_sensor struct
*  out - 1 (true) if the calculated CRC8 is the same as the CRC8 byte that was recieved from the sensor
*        0 (false) if the calculated CRC8 is different from the CRC8 byte that was recieved from the sensor
*/
bool aht20_get_humidity(struct aht20_sensor *sensor);

/* Gets a measurement from the sensor and performs calculationsmfor temperature (in Celcuis).
*  Checks CRC of data received.
*
*  in sensor - pointer to aht20_sensor struct
*  out - 1 (true) if the calculated CRC8 is the same as the CRC8 byte that was recieved from the sensor
*        0 (false) if the calculated CRC8 is different from the CRC8 byte that was recieved from the sensor
*/
bool aht20_get_temp(struct aht20_sensor *sensor);


/* Sends a command to the AHT20 sensor to perform a soft reset, followed by a 20ms wait.
*
*  in sensor - pointer to aht20_sensor struct that provides handle to send reset command
*  out - None
*/
void aht20_soft_reset(struct aht20_sensor * sensor);

#endif /* !AHT20_H */
