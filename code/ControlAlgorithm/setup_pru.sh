#!/bin/bash
modprobe uio_pruss extram_pool_sz=0x160000
./devicetree/build ./devicetree/BB-CAMERA-00A0
#./devicetree/build ./devicetree/BB-BONE-PRU-01-00A0
cp ./devicetree/BB-CAMERA-00A0.dtbo /lib/firmware
#cp ./devicetree/BB-BONE-PRU-01-00A0.dtbo /lib/firmware
echo BB-CAMERA > /sys/devices/bone_capemgr.*/slots
echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.*/slots
echo 30 > /sys/class/gpio/export
echo 60 > /sys/class/gpio/export
ldconfig /usr/xenomai/lib
umount /tmp/quadtempfs
rmdir /tmp/quadtempfs
mkdir /tmp/quadtempfs
mount -t tmpfs -o size=20m tmpfs /tmp/quadtempfs
mkdir /root/1314-BeagleBone-Quadcopter/code/ControlTower/ramfs

mount -t tmpfs -o size=20m tmpfs /root/1314-BeagleBone-Quadcopter/code/ControlTower/ramfs
echo `pwd`
