
all:  vc knobtest poc poc2 poc3

vc: VoltageCatcher.cpp
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi VoltageCatcher.cpp -o vc

knobtest: knobtest.cpp
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi knobtest.cpp -o knobtest

poc: continuous.poc.c
	gcc -O2 continuous.poc.c -o poc

poc2: continuous.wpi.c
	gcc -O2 -lwiringPi continuous.wpi.c -o poc2

poc3: continuous.ads.c
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi continuous.ads.c -o poc3

clean:
	rm -f vc knobtest poc*

