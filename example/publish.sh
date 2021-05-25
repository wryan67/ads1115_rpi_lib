#!/bin/ksh

make
[ $? != 0 ] && exit

scp vc rotor2:/home/pi/projects/ads1115_rpi_lib/example
scp poc2 rotor2:/home/pi/projects/ads1115_rpi_lib/example
scp poc3 rotor2:/home/pi/projects/ads1115_rpi_lib/example
