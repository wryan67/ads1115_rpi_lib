
all:  poc1 poc2 poc3


poc1: continuous.poc.c
	gcc -O2 continuous.poc.c -o poc1

poc2: continuous.wpi.c
	gcc -O2 -lwiringPi continuous.wpi.c -o poc2

poc3: continuous.ads.c
	gcc -O2 -lwiringPi -lwiringPiADS1115rpi continuous.ads.c -o poc3

clean:
	rm -f poc*

