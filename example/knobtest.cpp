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
int   gain = 0;

unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}

bool usage() {
    fprintf(stderr, "usage: knobtest [-a address] [-g gain] [-v vRef]\n");
    fprintf(stderr, "a = hex address of the ads1115 chip\n");
    fprintf(stderr, "v = refrence voltage\n");
    fprintf(stderr, "g = gain; default=0; see chart:\n");
    fprintf(stderr, "    0 = +/- %5.3f volts\n", 6.144);
    fprintf(stderr, "    1 = +/- %5.3f volts\n", 4.096);
    fprintf(stderr, "    2 = +/- %5.3f volts\n", 2.048);
    fprintf(stderr, "    3 = +/- %5.3f volts\n", 1.024);
    fprintf(stderr, "    4 = +/- %5.3f volts\n", 0.512);
    fprintf(stderr, "    5 = +/- %5.3f volts\n", 0.256);

    return false;
}

bool commandLineOptions(int argc, char **argv) {
	int c, index;

	while ((c = getopt(argc, argv, "a:g:v:")) != -1)
		switch (c) {
			case 'a':
				sscanf(optarg, "%x", &ADS1115_ADDRESS);
				break;
			case 'g':
				sscanf(optarg, "%d", &gain);
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
    return 1;
  }

  printf("use -h to get help on command line options\n");
  printf("accessing ads1115 chip on i2c address 0x%02x\n", ADS1115_ADDRESS);
  int ADS1115_HANDLE = getADS1115Handle(ADS1115_ADDRESS);


// o = operation mode
// x = mux
// g = gain
// m = mode
//                oxxx gggm
// default 0x8583 1000 0101 1000 0011
//                1111 0101 1000 0011
//                1111 0101 1000 0011



  printf("Timestamp       Delta %12s %12s %12s %12s\n", "A0", "A1", "A2", "A3"); 

  float max=getADS1115MaxGain(gain);

  while (true) {
    float volts[4];

    int startTime=currentTimeMillis();
    for (int i=0;i<4;++i) {
      float v=readVoltageSingleShot(ADS1115_HANDLE, i, gain);
      volts[i]=v;
    }

    long long now=currentTimeMillis();
    int elapsed = now - startTime;

    printf("%lld %7d %12.6f %12.6f %12.6f %12.6f\r", 
            now, elapsed, volts[0], volts[1], volts[2], volts[3]);

    for (int i=0;i<4;++i) {
      if (volts[i]<=(-max) || volts[i]>=max) {
        printf("\n");
        break;
      }
    }
    fflush(stdout);
  }
}

