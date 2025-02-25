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

/* Includes */
#include "AHT20.h"

/* Defines */

// I2C address of AHT20 sensor
#define AHT20_I2C_ADDR  0x38

// Length of sensor messages and commands
#define SENSOR_STATE_LEN 1
#define SENSOR_INIT_LEN  3
#define SENSOR_TRIG_LEN  3
#define SENSOR_RESET_LEN 1

/* Location of bytes in the measurement read from the AHT20
*
*  XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
*  -------- -------- -------- -------- -------- -------- --------
*   State   Humidity Humidity Humidity Temp     Temp     CRC
*                              & Temp
*/
#define STATUS_BYTE     0
#define HUMID_BYTE1     1
#define HUMID_BYTE2     2
#define HUMID_BYTE3     3
#define TEMP_BYTE1      3
#define TEMP_BYTE2      4
#define TEMP_BYTE3      5
#define CRC_BYTE        6

// Location of bits within the state byte
#define CALIBRATE_EN_BIT 3
#define BUSY_BIT         7

/* Static Variables */
static const char init_message[SENSOR_INIT_LEN] = { 0xbe, 0x08, 0x00 };
static const char trigger_message[SENSOR_TRIG_LEN] = { 0xac, 0x33, 0x00 };
static const char reset_message[SENSOR_RESET_LEN] = { 0xba };

/* Static Functions */
static unsigned char crc8(char *data, int len);
static void get_measurement(struct aht20_sensor *sensor);

/* Calculates CRC (based on https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c)
*
*  in data - pointer to array of bytes that will be used to calculate CRC8
*  in len - number of bytes used to calculate CRC8 (length of data)
*  out - CRC8 value
*/
static unsigned char crc8(char *data, int len)
{
    unsigned char crc = 0xff;
    unsigned char i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if ((crc & 0x80) != 0)
                crc = (unsigned char)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }

    return crc;
}

bool crc_check(struct aht20_sensor *sensor)
{
    unsigned char crc_calc;

    if(sensor == NULL)
    {
        return false;
    }

    crc_calc = crc8(sensor->buf, (SENSOR_DATA_LEN-1));

    if((crc_calc != sensor->buf[CRC_BYTE]) && (sensor->buf[CRC_BYTE] != 0xFF))
    {
        return false;
    }

    return true;
}

/* Sends a command to the sensor to trigger a measurement, then waits for the 
*  measurement to finish. Once the measurement is done, the data is stored 
*  in sensor->buf.
*
*  in sensor - pointer to aht20_sensor struct to provide I2C handle and buffer
*              to store data
*  out - None
*/
static void get_measurement(struct aht20_sensor *sensor)
{
   unsigned char status;

   lgI2cWriteDevice(sensor->lg_I2C_handle, trigger_message, SENSOR_TRIG_LEN);

   sleep(0.08);

   // Clear buffer and read state byte from sensor
   memset(sensor->buf, 0, SENSOR_DATA_LEN *sizeof(sensor->buf[0]));
   lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_STATE_LEN);

   // Check if sensor is busy taking measurement
   status = (1U << BUSY_BIT) & sensor->buf[0];

   while(status)
   {
       lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_STATE_LEN);

       status = (1U << BUSY_BIT) & sensor->buf[0];
   }

   // Clear buffer
   memset(sensor->buf, 0, SENSOR_DATA_LEN *sizeof(sensor->buf[0]));

   // Read data bytes form sensor
   lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_DATA_LEN);

}


void aht20_init(struct aht20_sensor *sensor)
{
   unsigned char status;

   sensor->lg_I2C_handle = lgI2cOpen(sensor->I2C_dev, AHT20_I2C_ADDR, 0);

   lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_STATE_LEN);
   status = (1U << CALIBRATE_EN_BIT) & sensor->buf[STATUS_BYTE];

   if(status == 0)
   {
      lgI2cWriteDevice(sensor->lg_I2C_handle, init_message,  SENSOR_INIT_LEN);

      sleep(0.01);
   }
}

bool aht20_get_all_data(struct aht20_sensor *sensor)
{
   int temp;
   int humidity;

   get_measurement(sensor);

   temp = (int)((sensor->buf[3] & 0x0F) << 16) | (int)(sensor->buf[4] << 8) | (int)sensor->buf[5];

   sensor->temperature = (temp/pow(2,20)) * 200-50;

   humidity = (int)(sensor->buf[1] << 12) | (int)(sensor->buf[2] << 4) | (int)((sensor->buf[3] & 0xF0) >> 4);
   sensor->humidity = humidity * 100 / pow(2,20);

   return crc_check(sensor);
}

bool aht20_get_temp(struct aht20_sensor *sensor)
{
   int temp;

   get_measurement(sensor);

   temp = (int)((sensor->buf[3] & 0x0F) << 16) | (int)(sensor->buf[4] << 8) | (int)sensor->buf[5];

   sensor->temperature = (temp/pow(2,20)) * 200-50;

   return crc_check(sensor);
}

bool aht20_get_humidity(struct aht20_sensor *sensor)
{
   int humidity;

   get_measurement(sensor);

   humidity = (int)(sensor->buf[1] << 12) | (int)(sensor->buf[2] << 4) | (int)((sensor->buf[3] & 0xF0) >> 4);
   sensor->humidity = humidity * 100 / pow(2,20);

   return crc_check(sensor);
}

void aht20_soft_reset(struct aht20_sensor *sensor)
{
    lgI2cWriteDevice(sensor->lg_I2C_handle, reset_message,  SENSOR_RESET_LEN);

    sleep(0.02);
}
