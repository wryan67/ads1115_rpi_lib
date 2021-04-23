#pragma once

#ifdef __cplusplus
  extern "C"
{
#endif

extern float gainMax[8];
float readVoltage(int handle, int pin, int gain);

#ifdef __cplusplus
}
#endif
