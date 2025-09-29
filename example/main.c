#include <AHT20.h>
#include <stdio.h>

#define I2C_BUS 1

int main(void)
{
   struct aht20_sensor sensor;
   bool status;

   // Set the I2C device/bus to use
   sensor.I2C_dev = I2C_BUS;

   status = aht20_init(&sensor);
   if(status == false)
   {
        fprintf(stderr, "Sensor failed initialization\n");
        return EXIT_FAILURE;
   }
       

   while(1)
   {

       status = aht20_get_temp(&sensor);
       if(status == false)
       {
            fprintf(stderr, "Getting temperature failed\n");
       }

       status = aht20_get_humidity(&sensor);
       if(status == false)
       {
            fprintf(stderr, "Sensor humidity failed\n");
       }

       printf("Temp: %.2fC %.2fF\n", sensor.temperature, C_TO_F(sensor.temperature));
       printf("Relative Humidity: %.2f%%\n", sensor.humidity);
       sleep(5);
   }

   return EXIT_SUCCESS;
}
