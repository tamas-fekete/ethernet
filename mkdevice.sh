#! /bin/bash


ip tuntap add dev tap0 mode tap
brctl addbr br0
brctl addif br0 eth0
brctl addif br0 tap0
ifconfig br0 up
ifconfig tap0 up
