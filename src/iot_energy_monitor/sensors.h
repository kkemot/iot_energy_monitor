#pragma once

// Sampling time takes 3-4s,
// min time for sampling delay is 15s
// so we  get 3 samples from sensors
// 15/4 -> 3 samples sampling rate ~20% (3samples for 15s)
#define SAMPLING_RATE 20

void init_sensors(void);
void energyMeter_read(void);
void energyMeter_clearBuffers(void);

