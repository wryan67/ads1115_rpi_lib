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

#include <vector>

using namespace std;


int ADS1115_ADDRESS=0x48;
int ADS1115_HANDLE=-1;

int   debug = 0;
int   channel = 0;
int   gain = 0;
float seconds = -1;
int   sps = 0;
int   ok2run = 1;

long long sample=-1;
long long sampleStart=0;
long long lastSampleTimestamp=0;

unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}


void intHandler(int dummy) {
  fprintf(stderr,"\ninterrupt received; shutting down...\n");

  ok2run = 0;
}

class Sample {
  public:
  long long sample;
  long long timestamp;
  long long delta;
  long long offset;
  float     volts;
};

vector<Sample*> samples;


bool usage() {
    fprintf(stderr, "usage: vc [-a address] [-g gain] -s seconds -f sps\n");
    fprintf(stderr, "a = hex address of the ads1115 chip\n");
    fprintf(stderr, "c = channel\n");
    fprintf(stderr, "v = refrence voltage\n");
    fprintf(stderr, "s = seconds to run capture (can be fractional)\n");
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

	while ((c = getopt(argc, argv, "a:c:dg:v:s:f:")) != -1)
		switch (c) {
			case 'a':
				sscanf(optarg, "%x", &ADS1115_ADDRESS);
				break;
			case 'c':
				sscanf(optarg, "%d", &channel);
				break;
			case 'd':
				debug=1;
				break;
			case 'f':
				sscanf(optarg, "%d", &sps);
				break;
			case 'g':
				sscanf(optarg, "%d", &gain);
				break;
			case 's':
				sscanf(optarg, "%f", &seconds);
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

    if (!isValidSPS(sps)) {
        return usage();
    }

    if (!isValidGain(gain)) {
        return usage();
    }

    if (seconds<=0) {
      return usage();
    }

	return true;
}

void getSample() {

  if (!ok2run) {
    return;
  }
  ++sample;
  long long now     = currentTimeMillis();

  if (lastSampleTimestamp == 0) {
    lastSampleTimestamp = now;
  }

  long long offset    = now - sampleStart;
  long long delta     = now - lastSampleTimestamp;
  lastSampleTimestamp = now;
  float volts         = readVoltage(ADS1115_HANDLE);

  Sample *s = new Sample();

  s->sample     = sample;
  s->timestamp  = now;
  s->delta      = delta;
  s->offset     = offset;
  s->volts      = volts;

  samples.push_back(s);
}


int main(int argc, char **argv) {

  if (!commandLineOptions(argc, argv)) {
    return 1;
  }

  if (wiringPiSetup()!=0) {
    fprintf(stderr,"cannot initialize WiringPi\n");
    return 1;
  }
  adsDebug(debug);

  fprintf(stderr,"use -h to get help on command line options\n");
  fprintf(stderr,"accessing ads1115 chip on i2c address 0x%02x\n", ADS1115_ADDRESS);
  ADS1115_HANDLE = wiringPiI2CSetup(ADS1115_ADDRESS);

  signal(SIGINT, intHandler);


  fprintf(stderr, "sps=%d; gain=[+/-]%.3f; seconds=%.2f\n", 
           getADSampleRate(sps), getADS1115MaxGain(gain), seconds);
  
  printf("Sample,Timestamp,Delta,TimeOffset,A%d\n",channel); 

  setADS1115ContinuousMode(ADS1115_HANDLE, channel, gain, sps);


  sampleStart=currentTimeMillis();
  long long end = sampleStart + (seconds * 1000.0);
  fprintf(stderr,"now:  %lld\n",sampleStart);
  fprintf(stderr,"end:  %lld\n",end);

  wiringPiISR(2,INT_EDGE_FALLING, getSample);
  
  while (ok2run && (seconds<0 || currentTimeMillis()<end)) {
    usleep(1000);
  }

  adsReset(ADS1115_HANDLE);
  fprintf(stderr,"exit: %lld; samples taken: %lld\n", currentTimeMillis(), sample);

  for (Sample *s: samples) {
      printf("%lld,%lld,%lld,%lld,%f\n", s->sample, s->timestamp, s->delta, s->offset, s->volts);
  }
}

