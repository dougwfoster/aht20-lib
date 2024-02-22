#include <AHT20.h>
#include <stdio.h>

#define I2C_BUS 1

int main(void)
{
   struct aht20_sensor sensor;
   bool status;

   // Set the I2C device/bus to use
   sensor.I2C_dev = I2C_BUS;

   aht20_init(&sensor);

   while(1)
   {

       status = aht20_get_temp(&sensor);
       if(status == false)
       {
            printf("CRC failed while getting temperature\n");
       }

       aht20_get_humidity(&sensor);
       if(status == false)
       {
            printf("CRC failed while getting humidity\n");
       }

       printf("Temp: %dC %dF\n", sensor.temperature, C_TO_F(sensor.temperature));
       printf("Relative Humidity: %.2f%%\n", sensor.humidity);
       sleep(5);
   }

   return 0;
}
