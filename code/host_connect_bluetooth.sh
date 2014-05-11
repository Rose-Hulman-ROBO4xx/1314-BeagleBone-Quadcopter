#!/bin/sh

# Make sure to have rfcomm loaded
sudo rfcomm release $1
sudo killall rfcomm
sudo killall pppd
sudo modprobe rfcomm

# Turn on and reset bluetooth dongle
sudo hciconfig $1 up
sudo hciconfig $1 reset

# Accept incoming connections (in background)
#sudo rfcomm connect $1 00:1B:DC:0F:40:CC 11 &
sudo rfcomm connect $1 00:15:83:4C:0B:F2 11 &
# Loop forever
#while true
#do
  # Wait for our socket to pop in
  while [ ! -c /dev/rfcomm0 ]
  do
    sleep 5
  done

  # Present a login shell
#  getty -n -l /bin/bash /dev/rfcomm0
#done
sudo pppd proxyarp mtu 1280 persist nodeflate noauth lcp-echo-interval 10 crtscts lock 10.10.1.1:10.10.1.2 /dev/rfcomm0 1000000000000


#allow the new ppp0 device to have internet forwarding
sudo sysctl net.ipv4.ip_forward=1
sudo sysctl net.ipv6.conf.default.forwarding=1
sudo sysctl net.ipv6.conf.all.forwarding=1
sudo sysctl net.ipv4.conf.all.forwarding=1
sudo iptables -t nat -A POSTROUTING -o wlp3s0 -j MASQUERADE
sudo iptables -A FORWARD -i ppp0 -o wlp3s0 -j ACCEPT
sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT


