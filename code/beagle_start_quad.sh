#~/usr/bin/env bash
TERM=xterm
SHELL=/bin/bash
SSH_TTY=/dev/pts/0
USER=root
PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin
PWD=/home/root
EDITOR=/bin/vi
NODE_PATH=/usr/lib/node_modules
PS1=\[`if [[ $? = 0 ]]; then echo '\e[32m\u@beaglebone\e[0m'; else echo '\e[31m\u@beaglebone\e[0m' ; fi`:\w\n$ 
SHLVL=1
HOME=/home/root
LOGNAME=root


cd /home/root/NewCopterRepo/code/i2c_bitbang/pru_sw
./setup_pru.sh
cd example_apps
cd bin
nice -n -20 ./control_alg &
netcat -l -p 1234 > asdf

