#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <byteswap.h>

#include "ads1115rpi.h"

float adsMaxGain[8] = {
  6.144,
  4.096,
  2.048,
  1.024,
  0.512,
  0.256,
  0.256,
  0.256
};

float adsSPS[8] = {
    // 8 SPS, 16 SPS, 32 SPS, 64 SPS, 128 SPS, 250 SPS, 475 SPS, or 860
    // 0       1       2       3        4        5        6           7
       8,     16,     32,     64,     128,     250,     475,        860
};

float getADS1115MaxGain(int gain) {
    if (gain<0) {
        return -1;
    } 
    if (gain>7) {
        return -1;
    }
    return adsMaxGain[gain];
}


int isValidSPS(int sps) {
    if (sps<0) {
        return 0;
    } 
    if (sps>7) {
        return 0;
    }
    return adsSPS[sps];
}



// o = operation mode
// x = mux (channel)
// g = gain
// m = mode (conversation mode) 

// d = data rate (default=128 sps)
// c = compare mode (default=traditional)
// p = comparator polarity (default=active-low)
// l = latching comparator (default=non-latching)
// q = comparator queue (default=disabled)
//
//                oxxx gggm dddc plqq
// default 0x8583 1000 0101 1000 0011
//                1111 0101 1000 0011
//                1111 0101 1000 0011


int getConfigConfig(struct adsConfig config) {
    int high=0;
    int low=0;

    high |= (0x01 && config.status)             << 7;  // 1 bit   
    high |= (0x01 && config.mux)                << 6;  // 1 bit   
    high |= (0x03 && config.channel)            << 4;  // 2 bits
    high |= (0x07 && config.gain)               << 1;  // 3 bits
    high |= (0x01 && config.operationMode)      << 0;  // 1 bit

    low |= (0x01 && config.dataRate)            << 5;  // 3 bit
    low |= (0x07 && config.compareMode)         << 4;  // 1 bit
    low |= (0x07 && config.comparatorPolarity)  << 3;  // 1 bits
    low |= (0x01 && config.latchingComparator)  << 2;  // 1 bit  
    low |= (0x01 && config.comparatorQueue)     << 0;  // 2 bits  

    return high << 8 | low;
}

int getSingeShotSingleEndedConfig(int pin, int gain) {
    int high = 1 << 7 | 1 << 6 | 1;
    high |= pin << 4;
    high |= gain << 1;

    int low = 0x83;
    return high << 8 | low;
}

float readVoltage(int handle, int pin, int gain) {
    int16_t  rslt = 0;
    uint16_t  config = getSingeShotSingleEndedConfig(pin, gain);

    wiringPiI2CWriteReg16(handle, 0x01, __bswap_16(config));
    delay(1);


    rslt = __bswap_16(wiringPiI2CReadReg16(handle, 0x01));
    while ((rslt & 0x8000) == 0) {  // wait for data ready
        delay(1);
        rslt = __bswap_16(wiringPiI2CReadReg16(handle, 0x01));
    }

    rslt = __bswap_16(wiringPiI2CReadReg16(handle, 0x00));

    // if (rslt > 32767) {
    //     rslt = 32767;
    // }

    return adsMaxGain[gain] * rslt / 32767.0;
}


