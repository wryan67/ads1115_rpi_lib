
all:  vc knobtest poc

vc: VoltageCatcher.cpp
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi VoltageCatcher.cpp -o vc

knobtest: knobtest.cpp
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi knobtest.cpp -o knobtest

poc: continuous.poc.c
	gcc -O2 continuous.poc.c -o poc

clean:
	rm -f vc knobtest poc

