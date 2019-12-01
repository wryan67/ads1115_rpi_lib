#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <byteswap.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>


#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <ads1115rpi.h>


int ADS1115_ADDRESS=0x48;

float vRef = 5.0;

unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}

bool usage() {
	fprintf(stderr, "usage: knobtest [-v vRef] [-a address]\n");
	fprintf(stderr, "a = hex address of the ads1115 chip\n");
	fprintf(stderr, "v = refrence voltage\n");

	return false;
}

bool commandLineOptions(int argc, char **argv) {
	int c, index;

	if (argc < 2) {
		return usage();
	}

	while ((c = getopt(argc, argv, "a:v:")) != -1)
		switch (c) {
			case 'a':
				sscanf(optarg, "%x", &ADS1115_ADDRESS);
				break;
			case 'v':
				sscanf(optarg, "%f", &vRef);
				break;
			case '?':
				if (optopt == 'm' || optopt=='t')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr,	"Unknown option character \\x%x.\n", optopt);

				return usage();
			default:
				abort();
		}

	
//	for (int index = optind; index < argc; index++)
//		printf("Non-option argument %s\n", argv[index]);
	return true;
}


int main(int argc, char **argv)
{
  if (!commandLineOptions(argc, argv)) {
    return 1;
  }
  if (wiringPiSetup()!=0) {
    printf("cannot initialize WiringPi\n");
  }

  int handle = wiringPiI2CSetup(ADS1115_ADDRESS);


// o = operation mode
// x = mux
// g = gain
// m = mode
//                oxxx gggm
// default 0x8583 1000 0101 1000 0011
//                1111 0101 1000 0011
//                1111 0101 1000 0011

  while (true) {
    float volts[4];

    int startTime=currentTimeMillis();
    for (int i=0;i<4;++i) {
      float v=readVoltage(handle, i, 0);
      if (v>6) {
       v=0;
      }
      volts[i]=v;
    }

    long long cTime=currentTimeMillis();
    int elapsed = cTime - startTime;

    printf("%lld %4d %8.2f %8.2f %8.2f %8.2f\r", cTime, elapsed, volts[0], volts[1], volts[2], volts[3]);
    if (volts[1]<0 || volts[1]>5.1) {
      printf("\n");
    }

    fflush(stdout);
  }
}

