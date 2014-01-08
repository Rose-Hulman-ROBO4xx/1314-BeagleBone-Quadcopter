Quadcopter Server README
========================

Necessary Hardware
------------------
1. BeagleBone Black running the latest image (9/4/2013 or later)
2. MPU 6050 six axis accelerometer and gyro
3. UWN 100 or UWN 200 USB WiFi Dongle
4. Wireless access point

Additional components such as wires and a power source may be helpful.

Necessary Software
------------------
1. Node.js
2. Various node packaged modules (I2C and MPU6050)
3. JQuery and flot
4. Source code files

All files except the installation of Node are available on this repo.

Installation
------------
1. Install Node.js
2. Install NPM
3. Install I2C and MPU6050 from NPM
4. Download all source to the correct directory

Useage
------
1. Connect the BBB to your wireless access point
2. Connect the host to the same network
3. Navigate to the proper IP address of the BBB, port 1337
4. Enjoy watching your data in real time

### Credits ###
Mike McDonald and Matt Skorina
Rose-Hulman ECE 497 Fall 2013
More detailed instructions for this project are available on the eLinux.org Wiki, located [here](http://elinux.org/ECE497_Project_Quadcopter_Server).