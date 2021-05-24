#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <byteswap.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <ads1115rpi.h>


int ADS1115_ADDRESS=0x48;
int ADS1115_HANDLE=-1;

int   channel = 0;
int   gain = 0;
int   seconds = -1;
int   sps = 0;
int   ok2run = 1;

long long sample=-1;
long long sampleStart=0;

unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}


void intHandler(int dummy) {
  stopContinuous(ADS1115_HANDLE);

  fprintf(stderr,"\ninterrupt received; shutting down...\n");
  ok2run = 0;
}


bool usage() {
    fprintf(stderr, "usage: vc [-a address] [-g gain] -s seconds -f sps\n");
    fprintf(stderr, "a = hex address of the ads1115 chip\n");
    fprintf(stderr, "c = channel\n");
    fprintf(stderr, "v = refrence voltage\n");
    fprintf(stderr, "s = seconds to run capture\n");
    fprintf(stderr, "f = samples per second\n");
    for (int i=0;i<8;++i) {
      fprintf(stderr, "    %d = %3d sps\n", i, getADSampleRate(i));
    }
    
    fprintf(stderr, "g = gain; default=0; see chart:\n");
    for (int i=0;i<6;++i) {
      fprintf(stderr, "    %d = +/- %5.3f volts\n", i, getADS1115MaxGain(i));
    }

    return false;
}

bool commandLineOptions(int argc, char **argv) {
	int c, index;

	while ((c = getopt(argc, argv, "a:c:g:v:s:f:")) != -1)
		switch (c) {
			case 'a':
				sscanf(optarg, "%x", &ADS1115_ADDRESS);
				break;
			case 'c':
				sscanf(optarg, "%d", &channel);
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

    if (sps<1) {
        return usage();
    }

	return true;
}

void getSample() {
  ++sample;
  long long now = currentTimeMillis();

  float volts = readVoltage(ADS1115_HANDLE);

  long long offset=now - sampleStart;

  printf("%lld,%lld,%f,%f,%f,%f\n", sample, now, offset, volts[0], volts[1], volts[2], volts[3]);


  fprintf(stderr,"volts=%7.3f\n",volts);
}

int main(int argc, char **argv) {

  if (!commandLineOptions(argc, argv)) {
    return 1;
  }
  if (wiringPiSetup()!=0) {
    fprintf(stderr,"cannot initialize WiringPi\n");
    return 1;
  }

  fprintf(stderr,"use -h to get help on command line options\n");
  fprintf(stderr,"accessing ads1115 chip on i2c address 0x%02x\n", ADS1115_ADDRESS);
  ADS1115_HANDLE = wiringPiI2CSetup(ADS1115_ADDRESS);

  signal(SIGINT, intHandler);


  fprintf(stderr, "sps=%d\n", getADSampleRate(sps));
  
  printf("Sample,Timestamp,TimeOffset,A%d\n",channel); 

  struct adsConfig config;

  config.status             =   0;
  config.mux                =   1;  // reference channel to ground
  config.channel            =   channel;
  config.gain               =   gain;
  config.operationMode      =   0;
  config.dataRate           =   sps;
  config.compareMode        =   1;
  config.comparatorPolarity =   0;
  config.latchingComparator =   0;
  config.comparatorQueue    =   3;

  setADS1115Config(ADS1115_HANDLE, config);

 // set hi/lo threshold register
  wiringPiI2CWriteReg16(ADS1115_HANDLE, ADS1115_HiThresholdRegister, 0xff);
  wiringPiI2CWriteReg16(ADS1115_HANDLE, ADS1115_LoThresholdRegister, 0x00);

  int conversionRegister=0;
  wiringPiI2CWriteReg16(ADS1115_HANDLE, 0x01, __bswap_16(conversionRegister));

  wiringPiISR(2,INT_EDGE_FALLING, getSample);

  sampleStart=currentTimeMillis();
  
  while (ok2run && (seconds<0 || currentTimeMillis()<end)) {
    sleep(1);
  }




}

