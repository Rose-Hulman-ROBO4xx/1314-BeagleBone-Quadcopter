#!/bin/bash
cd app_loader/interface/
gcc -I. -Wall -I../include   -c -fPIC -O3 -mtune=cortex-a8 -march=armv7-a -shared -o prussdrv.o prussdrv.c
gcc -shared -o libprussdrv.so prussdrv.o

sudo cp libprussdrv.so /usr/lib/
sudo cp ../include/*.h /usr/include/
