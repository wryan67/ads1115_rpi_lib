
all:  vc knobtest 

vc: VoltageCatcher.cpp
	g++ -O2 -lwiringPi -lwiringPiADS1115rpi VoltageCatcher.cpp -o vc

knobtest: knobtest.cpp
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi knobtest.cpp -o knobtest

clean:
	rm -f vc knobtest 

