#!/bin/bash

echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.*/slots
modprobe uio_pruss
echo 30 > /sys/class/gpio/export
echo 60 > /sys/class/gpio/export

