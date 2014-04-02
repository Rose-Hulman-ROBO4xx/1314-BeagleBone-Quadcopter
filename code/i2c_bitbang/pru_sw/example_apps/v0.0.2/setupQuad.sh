#!/bin/sh
# Script to start quad control program
cp rc.local /etc/rc.local
cp /home/ubuntu/quad/blackdts/am335x-boneblack.dtb /boot/uboot/dtbs/
chmod 777 /home/ubuntu/quad/startQuad.sh 
chmod 777 /home/ubuntu/quad/control/setenv.sh
chmod 777 /home/ubuntu/quad/control/control