#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <byteswap.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <ads1115rpi.h>


int ADS1115_ADDRESS=0x48;


unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}

int main(void)
{
    wiringPiSetup();

    int handle = wiringPiI2CSetup(ADS1115_ADDRESS);

    float range = 5.0;

// o = operation mode
// x = mux
// g = gain
// m = mode
//                oxxx gggm
// default 0x8583 1000 0101 1000 0011
//                1111 0101 1000 0011
//                1111 0101 1000 0011

    while (true) {
      float volts[4];
  
      int startTime=currentTimeMillis();
      for (int i=0;i<4;++i) {
        float v=readVoltage(handle, i, 0);
        if (v>6) {
         v=0;
        }
        volts[i]=v;
      }

      long long cTime=currentTimeMillis();
      int elapsed = cTime - startTime;

      printf("%lld %4d %8.2f %8.2f %8.2f %8.2f\r", cTime, elapsed, volts[0], volts[1], volts[2], volts[3]);
      if (volts[1]<0 || volts[1]>5.1) {
        printf("\n");
      }

      fflush(stdout);
    }
}

