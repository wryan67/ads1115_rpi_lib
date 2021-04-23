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
int   seconds = 0;
int   sps = 0;

unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}

bool usage() {
	fprintf(stderr, "usage: knobtest [-a address] [-g gain] [-v vRef] -s seconds -f sps\n");
	fprintf(stderr, "a = hex address of the ads1115 chip\n");
	fprintf(stderr, "v = refrence voltage\n");
    fprintf(stderr, "s = seconds to run capture\n");
    fprintf(stderr, "f = samples per second\n");
    
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

	while ((c = getopt(argc, argv, "a:g:v:s:f:")) != -1)
		switch (c) {
			case 'a':
				sscanf(optarg, "%x", &ADS1115_ADDRESS);
				break;
			case 'f':
				sscanf(optarg, "%d", &sps);
				break;
			case 'g':
				sscanf(optarg, "%d", &gain);
				break;
			case 's':
				sscanf(optarg, "%d", &seconds);
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

    if (seconds<1 || sps<1) {
        return usage();
    }

	return true;
}


int main(int argc, char **argv)
{
  if (!commandLineOptions(argc, argv)) {
    return 1;
  }
  if (wiringPiSetup()!=0) {
    fprintf(stderr,"cannot initialize WiringPi\n");
    return 1;
  }

  fprintf(stderr,"use -h to get help on command line options\n");
  fprintf(stderr,"accessing ads1115 chip on i2c address 0x%02x\n", ADS1115_ADDRESS);
  int handle = wiringPiI2CSetup(ADS1115_ADDRESS);


// o = operation mode
// x = mux
// g = gain
// m = mode
//                oxxx gggm
// default 0x8583 1000 0101 1000 0011
//                1111 0101 1000 0011
//                1111 0101 1000 0011



  long uperiod = 1000000 / sps;
  long period = 1000 / sps;
  long long sample=-1;
  long long end = currentTimeMillis() + (seconds * 1000);

  fprintf(stderr, "sps=%d period=%ld\n", sps, period);
  printf("Sample,Timestamp,A0,A1,A2,A3\n"); 

  while (currentTimeMillis()<end) {
    ++sample;
    float volts[4];

    long long sampleStart=currentTimeMillis();
    long long sampleEnd=sampleStart+period;

    if ((sampleStart%1000)==0) {
        fprintf(stderr,".");
    }

    for (int i=0;i<4;++i) {
      float v=readVoltage(handle, i, gain);
      if (v>adsMaxGain[gain]) {
       v=0;
      }
      volts[i]=v;
    }

    printf("%lld,%lld,%f,%f,%f,%f\n", sample, sampleStart, volts[0], volts[1], volts[2], volts[3]);

    long elapsed = currentTimeMillis() - sampleStart;
    long delay   = uperiod - elapsed;

    if (delay>10) {
        usleep(delay-7000);
    }
    // while (currentTimeMillis()<sampleEnd-7) {fprintf(stderr,"+%lld %lld\n",currentTimeMillis(),sampleEnd);}
  }
}

