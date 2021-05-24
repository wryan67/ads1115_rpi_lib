#pragma once



#ifdef __cplusplus
  extern "C"
{
#endif

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

float getADS1115MaxGain(int gain);
int   getADSsampleRate(int sps);
float readVoltage(int handle, int pin, int gain);
int   getConfig(struct adsConfig config);

#ifdef __cplusplus
}
#endif
