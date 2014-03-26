#!/bin/sh
dtc -O dtb -o am335x-boneblack.dtb -b 0  am335x-boneblack.dts
cp am335x-boneblack.dtb /boot/uboot/dtbs/
