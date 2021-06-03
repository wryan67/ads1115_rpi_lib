
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


#define BUFSIZE 256				// Typical charbuffer



#define ADS1115_ConversionRegister    0x00
#define ADS1115_ConfigurationRegister 0x01
#define ADS1115_LoThresholdRegister   0x02
#define ADS1115_HiThresholdRegister   0x03


struct adsConfig {
                                // bit
    int status;                 //    15  when writing: 0=no effect; 1=single shot
    int mux;                    //    14  0=see datasheet; 1=reference to ground
    int channel;                // 12-13  
    int gain;                   //  9-11  see table
    int operationMode;          //     8  0=continuous; 1=single-shot or power-down

    int dataRate;               //   5-7  see table
    int compareMode;            //     4  0=traditional (default); 1=window
    int comparatorPolarity;     //     3  0=active low(defualt);   1=active hight
    int latchingComparator;     //     2  0=non-latching(default); 1=latching
    int comparatorQueue;        //   0-1  0=assert after one conversion
                                //        1=assert after two conversions
                                //        2=assert after four conversions
                                //        3=disabled(default)
     
};

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
uint16_t config2int(struct adsConfig config) {
    uint16_t high=0;
    uint16_t low=0;

    high |= (0x01 & config.status)             << 7;  // 1 bit   
    high |= (0x01 & config.mux)                << 6;  // 1 bit   
    high |= (0x03 & config.channel)            << 4;  // 2 bits
    high |= (0x07 & config.gain)               << 1;  // 3 bits
    high |= (0x01 & config.operationMode)      << 0;  // 1 bit

    low |= (0x07 & config.dataRate)            << 5;  // 3 bit
    low |= (0x01 & config.compareMode)         << 4;  // 1 bit
    low |= (0x01 & config.comparatorPolarity)  << 3;  // 1 bits
    low |= (0x01 & config.latchingComparator)  << 2;  // 1 bit  
    low |= (0x03 & config.comparatorQueue)     << 0;  // 2 bits  

    return (high << 8)|low;
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


    struct adsConfig config;



    config.status             =   0;   // no effect
    config.mux                =   1;   // reference channel to ground
    config.channel            =   0;
    config.gain               =   0;
    config.operationMode      =   0;   // continuous conversion 
    config.dataRate           =   0;
    config.compareMode        =   0;   // traditional
    config.comparatorPolarity =   0;
    config.latchingComparator =   0;
    config.comparatorQueue    =   0;


	if (wiringPiI2CWriteReg16(fd, ADS1115_ConfigurationRegister, __bswap_16(config2int(config)))!=0) {
		perror("Write to register 1");
		exit(EXIT_FAILURE);
	}

   	float lastVolts = 999999;
	while (1) {
		val = __bswap_16(wiringPiI2CReadReg16(fd, ADS1115_ConversionRegister));

		float volts = 6.144 * val / 32767.0;			// volts per step

 		if (volts!=lastVolts) {
			printf(" val:   %6d     volts: %8.4f\r",val,volts);
  			lastVolts=volts;
 		}
		printf("\r");
	}
}
