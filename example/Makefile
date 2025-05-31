
all:  vc knobtest 

vc: VoltageCatcher.cpp
	g++ -O2 VoltageCatcher.cpp  -lwiringPi -lwiringPiADS1115rpi -o vc

knobtest: knobtest.cpp
	gcc -O2 knobtest.cpp  -lwiringPi -lwiringPiADS1115rpi -o knobtest

clean:
	rm -f vc knobtest 

