#!/bin/sh
# setup environment for serial ttyO1 output

#define macro
echo "define macro dm"
dm(){ devmem2 $1 w $2 | sed -n 3p | awk '{ print $6 }' ;}

echo "Setup envrionment for Pru ICSS"
modprobe uio_pruss && dm 0x44e00c00 0


