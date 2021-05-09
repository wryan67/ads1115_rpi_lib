#pragma once

#ifdef __cplusplus
  extern "C"
{
#endif

float getADS1115MaxGain(int gain);
float readVoltage(int handle, int pin, int gain);

#ifdef __cplusplus
}
#endif
