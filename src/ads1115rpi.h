#pragma once


#ifdef __cplusplus
  extern "C"
{
#endif


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

void  adsDebug(int boolean);

int   getADS1115Handle(int address);
void  setADS1115ContinuousMode(int handle, int channel, int gain, int sps);
void  setADS1115Config(int handle, struct adsConfig config);
void  setThreshold(int handle, int reg, uint16_t value);

float getADS1115MaxGain(int gain);
int   getADSampleRate(int sps);
float readVoltage(int handle);
float readVoltageSingleShot(int handle, int pin, int gain);
void  adsReset(int handle);

int isValidSPS(int sps);
int isValidGain(int gain);
int isDataReady(int handle);  // software check

#ifdef __cplusplus
}
#endif
