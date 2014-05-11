#!/bin/bash
start_dir=`pwd`
cd utils/pasm_source
./linuxbuild
cd ../../app_loader/interface/
make clean
make
