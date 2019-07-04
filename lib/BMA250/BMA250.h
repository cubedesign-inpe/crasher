// I2Cdev library collection - BMA250 I2C device class header file
// Based on BMA250 datasheet, 29/05/2008
// 01/18/2012 by Brian McCain <bpmccain@gmail.com>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2012-01-18 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2011 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef _BMA250_H_
#define _BMA250_H_

#include "I2Cdev.h"

#define BMA250_ADDRESS_00           0x18                // Default Address
#define BMA250_ADDRESS_01           0x19                // Used on the Atmel ATAVRSBIN1
#define BMA250_DEFAULT_ADDRESS      BMA250_ADDRESS_00

#define BMA250_RA_CHIP_ID           0x00
#define BMA250_RA_VERSION           0x01
#define BMA250_RA_X_AXIS_LSB        0x02
#define BMA250_RA_X_AXIS_MSB        0x03
#define BMA250_RA_Y_AXIS_LSB        0x04
#define BMA250_RA_Y_AXIS_MSB        0x05
#define BMA250_RA_Z_AXIS_LSB        0x06
#define BMA250_RA_Z_AXIS_MSB        0x07
#define BMA250_RA_TEMP_RD           0x08
#define BMA250_RA_INT_STATUS_0      0x09
#define BMA250_RA_INT_STATUS_1      0x0a
#define BMA250_RA_INT_STATUS_2      0x0b
#define BMA250_RA_INT_STATUS_3      0x0c
#define BMA250_RA_FIFO_STATUS       0x0e
#define BMA250_RA_PMU_RANGE         0x0f
#define BMA250_RA_PMU_BW            0x10
#define BMA250_RA_PMU_LPW           0x11
#define BMA250_RA_PMU_LOW_POWER     0x12
#define BMA250_RA_ACCD_HBW          0x13
#define BMA250_RA_SOFT_RESET        0x14
#define BMA250_RA_INT_EN_0          0x16
#define BMA250_RA_INT_EN_1          0x17
#define BMA250_RA_INT_EN_2          0x18
#define BMA250_RA_INT_MAP_0         0x19
#define BMA250_RA_INT_MAP_1         0x1a
#define BMA250_RA_INT_MAP_2         0x1b
#define BMA250_RA_INT_SRC           0x1e
#define BMA250_RA_INT_OUT_CTRL      0x20
#define BMA250_RA_INT_RST_LATCH     0x21
#define BMA250_RA_INT_0             0x22
#define BMA250_RA_INT_1             0x23
#define BMA250_RA_INT_2             0x24
#define BMA250_RA_INT_3             0x25
#define BMA250_RA_INT_4             0x26
#define BMA250_RA_INT_5             0x27
#define BMA250_RA_INT_6             0x28
#define BMA250_RA_INT_7             0x29
#define BMA250_RA_INT_8             0x2A
#define BMA250_RA_INT_9             0x2B
#define BMA250_RA_INT_A             0x2C
#define BMA250_RA_INT_B             0x2D
#define BMA250_RA_INT_C             0x2E
#define BMA250_RA_INT_D             0x2F
#define BMA250_RA_FIFO_CONFIG_0     0x30
#define BMA250_RA_PMU_SELF_TEST     0x32
#define BMA250_RA_TRIM_NVM_CTRL     0x33
#define BMA250_RA_SPI3_WDT          0x34
#define BMA250_RA_OFC_CTRL          0x36
#define BMA250_RA_OFC_SETTING       0x37
#define BMA250_RA_OFC_OFFSET_X      0x38
#define BMA250_RA_OFC_OFFSET_Y      0x39
#define BMA250_RA_OFC_OFFSET_Z      0x3A
#define BMA250_RA_TRIM_GP0          0x3B
#define BMA250_RA_TRIM_GP1          0x3C
#define BMA250_RA_FIFO_CONFIG_1     0x3D
#define BMA250_RA_FIFO_DATA         0x3F

#define BMA250_X_AXIS_LSB_BIT          7
#define BMA250_X_AXIS_LSB_LENGTH       2
#define BMA250_X_NEW_DATA_BIT          0

#define BMA250_Y_AXIS_LSB_BIT          7
#define BMA250_Y_AXIS_LSB_LENGTH       2
#define BMA250_Y_NEW_DATA_BIT          0

#define BMA250_Z_AXIS_LSB_BIT          7
#define BMA250_Z_AXIS_LSB_LENGTH       2
#define BMA250_Z_NEW_DATA_BIT          0

/* range and bandwidth */
#define BMA250_RANGE_2G                0x03
#define BMA250_RANGE_4G                0x05
#define BMA250_RANGE_8G                0x08
#define BMA250_RANGE_16G               0x0C

#define BMA250_BW_MIN_HZ               0x00
#define BMA250_BW_07_81HZ              0x08
#define BMA250_BW_15_63HZ              0x09
#define BMA250_BW_31_25HZ              0x0A
#define BMA250_BW_62_50HZ              0x0C
#define BMA250_BW_125HZ                0x0C
#define BMA250_BW_250HZ                0x0D
#define BMA250_BW_500HZ                0x0E
#define BMA250_BW_MAX_HZ               0x1F

/* mode settings */
#define BMA250_MODE_NORMAL             0
#define BMA250_MODE_SLEEP              1

class BMA250 {
    public:
        BMA250();
        BMA250(uint8_t address);
        
        void initialize();
        bool testConnection();
        void reset();

        // CHIP_ID register
        uint8_t getDeviceID();
        
        // VERSION register
        uint8_t getChipRevision();
        
        // AXIS registers
        void getAcceleration(int16_t* x, int16_t* y, int16_t* z);
        int16_t getAccelerationX();
        int16_t getAccelerationY();
        int16_t getAccelerationZ();
        bool newDataX();
        bool newDataY();
        bool newDataZ();
                
        // TEMP register
        int8_t getTemperature();
        
        // RANGE / BANDWIDTH registers
        uint8_t getRange();
        void setRange(uint8_t range);
        uint8_t getBandwidth();
        void setBandwidth(uint8_t bandwidth);
        
        // OFFS_GAIN registers
        
        // OFFSET registers
        
        private:
        uint8_t devAddr;
        uint8_t buffer[6];
        uint8_t mode;
};

#endif /* _BMA250_H_ */
