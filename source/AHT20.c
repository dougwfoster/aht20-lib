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
static bool get_measurement(struct aht20_sensor *sensor);

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

/* Uses sensor data to perform CRC check
*
*  in sensor - pointer to struct aht20_sensor to provide sensor data and CRC byte
*              to help validate calculated CRC
*
*  out - true or false based on if crc check was successfully performed
*/
bool crc_check(struct aht20_sensor *sensor)
{
    unsigned char crc_calc;

    if(sensor == NULL)
    {
        return false;
    }

    crc_calc = crc8(sensor->buf, (SENSOR_DATA_LEN-1));

    if(crc_calc != sensor->buf[CRC_BYTE])
    {
        return false;
    }

    return true;
}

/* Sleeps for a specified number of milliseconds. Continues with
*  sleep if interrupted.
*
*  in ms - time to be delayed in milliseconds
*
*  out - None
*/
static void delay_ms(unsigned ms)
{
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = (long)(ms * 1000000L);

   /* Wait the full time if sleep is interrupted */
    while (nanosleep(&req, &req) == -1 && errno == EINTR);
}

/* Sends a command to the sensor to trigger a measurement, then waits for the 
*  measurement to finish. Once the measurement is done, the data is stored
*  in sensor->buf.
*
*  in sensor - pointer to struct aht20_sensor to provide I2C handle and buffer
*              to store data
*  out - 1 (true) if getting measurment from sensor succeeds
*        0 (false) if getting measurement fails
*/
static bool get_measurement(struct aht20_sensor *sensor)
{
   unsigned char status;
   int result;

   result = lgI2cWriteDevice(sensor->lg_I2C_handle, trigger_message, SENSOR_TRIG_LEN);
   if(result < 0)
       return false;

   delay_ms(80);

   // Clear buffer and read state byte from sensor
   memset(sensor->buf, 0, SENSOR_DATA_LEN *sizeof(sensor->buf[0]));
   result = lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_STATE_LEN);
   if(result < 0)
       return false;

   // Check if sensor is busy taking measurement
   status = (1U << BUSY_BIT) & sensor->buf[0];

   while(status)
   {
       result = lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_STATE_LEN);
       if(result < 0)
           return false;

       status = (1U << BUSY_BIT) & sensor->buf[0];
   }

   // Clear buffer
   memset(sensor->buf, 0, SENSOR_DATA_LEN *sizeof(sensor->buf[0]));

   // Read data bytes from sensor
   result = lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_DATA_LEN);
   if(result < 0)
       return false;
   else
       return true;
}


bool aht20_init(struct aht20_sensor *sensor)
{
   unsigned char status;
   int result;

   if(sensor == NULL)
       return false;

   sensor->lg_I2C_handle = lgI2cOpen(sensor->I2C_dev, AHT20_I2C_ADDR, 0);
   if(sensor->lg_I2C_handle < 0)
       return false;


   result = lgI2cReadDevice(sensor->lg_I2C_handle, sensor->buf, SENSOR_STATE_LEN);
   if(result < 0)
       return false;

   status = (1U << CALIBRATE_EN_BIT) & sensor->buf[STATUS_BYTE];

   if(status == 0)
   {
      result = lgI2cWriteDevice(sensor->lg_I2C_handle, init_message,  SENSOR_INIT_LEN);
      if(result < 0)
          return false;

      delay_ms(10);
   }

   return true;
}

bool aht20_get_all_data(struct aht20_sensor *sensor)
{
   int temp;
   int humidity;
   bool result =  false;

   if(sensor == NULL)
       return result;

   result = get_measurement(sensor);
   if(result == false)
       return result;

   temp = (int)((sensor->buf[3] & 0x0F) << 16) | (int)(sensor->buf[4] << 8) | (int)sensor->buf[5];

   sensor->temperature = (temp/pow(2,20)) * 200-50;

   humidity = (int)(sensor->buf[1] << 12) | (int)(sensor->buf[2] << 4) | (int)((sensor->buf[3] & 0xF0) >> 4);
   sensor->humidity = humidity * 100 / pow(2,20);
   
   result = crc_check(sensor);
   return result;
}

bool aht20_get_temp(struct aht20_sensor *sensor)
{
   int temp;
   bool result =  false;

   if(sensor == NULL)
       return result;

   result = get_measurement(sensor);
   if(result == false)
       return result;

   temp = (int)((sensor->buf[3] & 0x0F) << 16) | (int)(sensor->buf[4] << 8) | (int)sensor->buf[5];

   sensor->temperature = (temp/pow(2,20)) * 200-50;

   result = crc_check(sensor);
   return result;
}

bool aht20_get_humidity(struct aht20_sensor *sensor)
{
   int humidity;
   bool result =  false;

   if(sensor == NULL)
       return result;
   
   result = get_measurement(sensor);
   if(result == false)
       return result;

   humidity = (int)(sensor->buf[1] << 12) | (int)(sensor->buf[2] << 4) | (int)((sensor->buf[3] & 0xF0) >> 4);
   sensor->humidity = humidity * 100 / pow(2,20);

   result = crc_check(sensor);
   return result;
}

bool aht20_soft_reset(struct aht20_sensor *sensor)
{
   int result;

   if(sensor == NULL)
       return false;

   result = lgI2cWriteDevice(sensor->lg_I2C_handle, reset_message,  SENSOR_RESET_LEN);
   if(result < 0)
       return false;

   delay_ms(20);
   return true;
}
