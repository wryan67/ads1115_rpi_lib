#!/bin/ksh

make
[ $? != 0 ] && exit

scp vc rotor2:/home/pi/projects/ads1115_rpi_lib/example