#pragma once

#ifdef __cplusplus
  extern "C"
{
#endif

extern float adsMaxGain[8];
float readVoltage(int handle, int pin, int gain);

#ifdef __cplusplus
}
#endif
