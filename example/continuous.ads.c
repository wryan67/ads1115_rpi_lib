
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <byteswap.h>


#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <ads1115rpi.h>

#define BUFSIZE 256				// Typical charbuffer



#define ADS1115_ConversionRegister    0x00
#define ADS1115_ConfigurationRegister 0x01
#define ADS1115_LoThresholdRegister   0x02
#define ADS1115_HiThresholdRegister   0x03



uint16_t configHi(struct adsConfig config) {
    uint16_t high=0;

    high |= (0x01 & config.status)             << 7;  // 1 bit   
    high |= (0x01 & config.mux)                << 6;  // 1 bit   
    high |= (0x03 & config.channel)            << 4;  // 2 bits
    high |= (0x07 & config.gain)               << 1;  // 3 bits
    high |= (0x01 & config.operationMode)      << 0;  // 1 bit


    return high;
}
uint16_t configLo(struct adsConfig config) {
    uint16_t low=0;

    low |= (0x07 & config.dataRate)            << 5;  // 3 bit
    low |= (0x01 & config.compareMode)         << 4;  // 1 bit
    low |= (0x01 & config.comparatorPolarity)  << 3;  // 1 bits
    low |= (0x01 & config.latchingComparator)  << 2;  // 1 bit  
    low |= (0x03 & config.comparatorQueue)     << 0;  // 2 bits  

    return low;
}

int main(int argc, char **argv) {

  	int fd;

	int ads_address = 0x48;		// 0x48 is the default address on i2c bus for ads1115

    	int16_t val;
		// open i2c device
	if ((fd = wiringPiI2CSetup(ads_address)) < 0) {
		printf("Error: Couldn't open device 0x%02x: %s\n", ads_address, strerror(errno));
		exit(EXIT_FAILURE);
	}


 	printf("connected to 0x%02x via wiringPi\n", ads_address);


    setADS1115ContinuousMode(fd, 0, 1, 5);

   	float lastVolts = 999999;
	while (1) {

        float volts = readVoltage(fd);

 		if (volts!=lastVolts) {
			printf(" volts: %8.4f\r",val,volts);
  			lastVolts=volts;
 		}
		printf("\r");
	}
}
