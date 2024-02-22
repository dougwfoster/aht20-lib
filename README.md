# AHT20 Sensor Library for Linux Single Board Computers

This library provides functions in C in order for Linux single board computers, 
such as the Raspberry Pi, to interact with the AHT20 temperature and humidity 
sensor from ASAIR via I2C. The provided functions rely on the I2C wrapper of the 
lgpio library, therefore, all devices supported by lgpio are supported by aht20-lib. 
The library has been tested on the Raspberry Pi 5.

# Install

## Prerequisites
The lgpio library must be downloaded and installed in order to use aht20-lib. Follow 
the steps below to download and install it.

    wget https://github.com/joan2937/lg/archive/master.zip
    unzip master.zip
    cd lg-master
    make
    sudo make install

More information on the lgpio library can be found [here](https://github.com/joan2937/lg/tree/master)

## Install aht20-lib

    git clone https://github.com/dougwfoster/aht20-lib.git
    cd aht20-lib
    make
    sudo make install

# Library Usage
Please see the **example** directory for an example of how to use the aht20-lib 
library to get data from the AHT20 sensor.

When linking the library with your own code, it is necessary to also link the lgpio 
library. The following is an example, with main.c containing the source code that uses the library.

    gcc main.c -laht20 -llgpio


