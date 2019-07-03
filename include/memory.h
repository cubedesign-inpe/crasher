
#ifndef MEMORY_H
#define MEMORY_H

// Magic number. Used to ensure the EEPROM has valid contents
// Change whenever addresses are modified or to reset values
#define MAGIC           0xA3

// Leds address in eeprom
#define EE_LEDS         1

// Max G address in eeprom: 0xAB A=mantissa, B=decimal
#define EE_MAXG         2

// Accelerometer calibration parameters
#define EE_ACC_BIAS_X   3 // and 4
#define EE_ACC_BIAS_Y   5 // and 6
#define EE_ACC_BIAS_Z   7 // and 8

// Cansat altitude datalog
#define EE_BARO         10
#define EE_BARO_SZ      (1024 - EE_BARO)

byte MemInit(void);
bool MemReady(void);

byte MemRead8(unsigned int addr);
byte MemRead16(unsigned int addr);
void MemWrite8(unsigned int addr, byte val);
void MemWrite16(unsigned int addr, int val);

bool MemWrite8NonBlock(unsigned int addr, byte val);


#endif



