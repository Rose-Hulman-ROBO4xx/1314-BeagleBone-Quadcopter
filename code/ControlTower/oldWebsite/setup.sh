ssh-keygen -f "/home/michael/.ssh/known_hosts" -R bone
ssh-keygen -f "/home/michael/.ssh/known_hosts" -R 192.168.7.2

echo quad2 > /etc/hostname

passwd

opkg update
opkg install git
opkg install openssh-keygen
cd .ssh
ssh-keygen -t rsa -C "yoder@rose-hulman.edu"

git clone git@github.com:mcdonamp/RH_ROBO_Quadcopter.git
