#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <byteswap.h>

#include "ads1115rpi.h"

static struct adsConfig configuration;

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


int isValidSPS(int sps) {
    if (sps<0) {
        return 0;
    } 
    if (sps>7) {
        return 0;
    }
    return adsSPS[sps];
}

int isValidGain(int gain) {
    if (gain<0) {
        return 0;
    } 
    if (gain>7) {
        return 0;
    }
    return adsMaxGain[gain];
}


int getADSampleRate(int sps) {
    return isValidSPS(sps);
}

float getADS1115MaxGain(int gain) {
    if (gain<0) {
        return -1;
    } 
    if (gain>7) {
        return -1;
    }
    return adsMaxGain[gain];
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


void  setADS1115ContinousMode(int handle, int channel, int gain, int sps) {
 
    struct adsConfig config;


    // int conversionRegister=0;
    // wiringPiI2CWriteReg16(ADS1115_HANDLE, 0x01, __bswap_16(conversionRegister));



    config.status             =   0;
    config.mux                =   1;  // reference channel to ground
    config.channel            =   channel;
    config.gain               =   gain;
    config.operationMode      =   0;     // continuous conversion 
    config.dataRate           =   sps;
    config.compareMode        =   1;
    config.comparatorPolarity =   0;
    config.latchingComparator =   0;
    config.comparatorQueue    =   0;


    setADS1115Config(handle, config);

    // wiringPiI2CWriteReg16(handle, ADS1115_ConversionRegister, 0x00);


 // set hi/lo threshold register
    wiringPiI2CWriteReg16(handle, ADS1115_HiThresholdRegister, 0xff);
    wiringPiI2CWriteReg16(handle, ADS1115_LoThresholdRegister, 0x00);

}

void setADS1115Config(int handle, struct adsConfig config) {
    int high=0;
    int low=0;

    stopContinuousMode(handle);
    delay(10);

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

    memcpy(&configuration,&config,sizeof(configuration));

    wiringPiI2CWriteReg16(handle, ADS1115_ConfigurationRegister, (low << 8)|high);
    delay(1);
}

void stopContinuousMode(int handle) {
    int high=0;
    int low=0;

    configuration.operationMode=1;

    high |= (0x01 && configuration.status)             << 7;  // 1 bit   
    high |= (0x01 && configuration.mux)                << 6;  // 1 bit   
    high |= (0x03 && configuration.channel)            << 4;  // 2 bits
    high |= (0x07 && configuration.gain)               << 1;  // 3 bits
    high |= (0x01 && configuration.operationMode)      << 0;  // 1 bit

    low |= (0x01 && configuration.dataRate)            << 5;  // 3 bit
    low |= (0x07 && configuration.compareMode)         << 4;  // 1 bit
    low |= (0x07 && configuration.comparatorPolarity)  << 3;  // 1 bits
    low |= (0x01 && configuration.latchingComparator)  << 2;  // 1 bit  
    low |= (0x01 && configuration.comparatorQueue)     << 0;  // 2 bits  


    wiringPiI2CWriteReg16(handle, ADS1115_ConfigurationRegister, (low << 8)|high);
    delay(1);
}

void setSingeShotSingleEndedConfig(int handle, int pin, int gain) {
    int high=0;
    int low=0;

    configuration.status=1;
    configuration.mux=1;
    configuration.channel=pin;
    configuration.gain=gain;
    configuration.operationMode=1;

    configuration.dataRate=4;
    configuration.compareMode=0;
    configuration.comparatorPolarity=0;
    configuration.latchingComparator=0;
    configuration.comparatorQueue=11;

    high |= (0x01 && configuration.status)             << 7;  // 1 bit   
    high |= (0x01 && configuration.mux)                << 6;  // 1 bit   
    high |= (0x03 && configuration.channel)            << 4;  // 2 bits
    high |= (0x07 && configuration.gain)               << 1;  // 3 bits
    high |= (0x01 && configuration.operationMode)      << 0;  // 1 bit

    low |= (0x01 && configuration.dataRate)            << 5;  // 3 bit
    low |= (0x07 && configuration.compareMode)         << 4;  // 1 bit
    low |= (0x07 && configuration.comparatorPolarity)  << 3;  // 1 bits
    low |= (0x01 && configuration.latchingComparator)  << 2;  // 1 bit  
    low |= (0x01 && configuration.comparatorQueue)     << 0;  // 2 bits  


    wiringPiI2CWriteReg16(handle, ADS1115_ConfigurationRegister, (low << 8)|high);
    delay(1);
}

float readSingleShotVoltage(int handle, int pin, int gain) {
    int16_t  rslt = 0;
    
    setSingeShotSingleEndedConfig(handle, pin, gain);

    rslt = __bswap_16(wiringPiI2CReadReg16(handle, ADS1115_ConfigurationRegister));
    while ((rslt & 0x8000) == 0) {  // wait for data ready
        delay(1);
        rslt = __bswap_16(wiringPiI2CReadReg16(handle, ADS1115_ConfigurationRegister));
    }

    return readVoltage(handle);
}


float readVoltage(int handle) {
    int16_t  rslt = 0;

    rslt = __bswap_16(wiringPiI2CReadReg16(handle, ADS1115_ConversionRegister));

    return adsMaxGain[configuration.gain] * rslt / 32767.0;
}
