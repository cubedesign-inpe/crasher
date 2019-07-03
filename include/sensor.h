

#ifndef SENSOR_H
#define SENSOR_H

byte SensorInit(void);
void SensorTest(void);
void SensorSetBias(int16_t x, int16_t y, int16_t z);
void SensorSetRange(byte Range);
float SensorG(void);
float SensorH(void);
void SensorAcc(int16_t* x, int16_t* y, int16_t* z, bool sensor);

#endif


