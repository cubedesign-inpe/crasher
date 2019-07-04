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

#include "BMA250.h"

/** Default constructor, uses default I2C address.
 * @see BMA250_DEFAULT_ADDRESS
 */
BMA250::BMA250() {
    devAddr = BMA250_DEFAULT_ADDRESS;
}

/** Specific address constructor.
 * @param address I2C address
 * @see BMA250_DEFAULT_ADDRESS
 * @see BMA250_ADDRESS_00
 */
BMA250::BMA250(uint8_t address) {
    devAddr = address;
}

/** Power on and prepare for general usage. This sets the full scale range of 
 * the sensor, as well as the bandwidth
 */
void BMA250::initialize() {
	setRange(BMA250_RANGE_2G);
	setBandwidth(BMA250_BW_125HZ);
}

/** Reset sensor
 */
void BMA250::reset() {
	I2Cdev::writeByte(devAddr, BMA250_RA_SOFT_RESET, 0xB6);
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
bool BMA250::testConnection() {
    byte id = getDeviceID();
    if(id == 0xFA) return true;  // BMA280  -> K mouser
    if(id == 0xFB) return true;  // BMA280  -> K mouser (datasheet Register 0x00 desc)
    if(id == 0xF9) return true;  // BMA250E -> I ali
    if(id == 0x03) return true;  // BMA250  -> 8 ali
    return false;
}

// CHIP_ID register
/** Get Device ID.
 * This register is used to verify the identity of the device (0b010).
 * @return Device ID (should be 2 dec)
 * @see BMA250_RA_CHIP_ID
 */
uint8_t BMA250::getDeviceID() {
    I2Cdev::readByte(devAddr, BMA250_RA_CHIP_ID, buffer);
    return buffer[0];
}


// VERSION register
/** Get Chip Revision number
 * @return Chip Revision
 * @see BMA250_RA_VERSION
 */
 uint8_t BMA250::getChipRevision() {
    I2Cdev::readByte(devAddr, BMA250_RA_VERSION, buffer);
    return buffer[0];
}
		
// AXIS registers
/** Get 3-axis accelerometer readings.
 * @param x 16-bit signed integer container for X-axis acceleration
 * @param y 16-bit signed integer container for Y-axis acceleration
 * @param z 16-bit signed integer container for Z-axis acceleration
 * @see BMA250_RA_Y_AXIS_LSB
 */

void BMA250::getAcceleration(int16_t* x, int16_t* y, int16_t* z) {
    I2Cdev::readBytes(devAddr, BMA250_RA_X_AXIS_LSB, 6, buffer);
    *x = ((((int16_t)buffer[1]) << 8) | (int16_t)buffer[0]) >> (8-BMA250_X_AXIS_LSB_LENGTH);
    *y = ((((int16_t)buffer[3]) << 8) | (int16_t)buffer[2]) >> (8-BMA250_Y_AXIS_LSB_LENGTH);
    *z = ((((int16_t)buffer[5]) << 8) | (int16_t)buffer[4]) >> (8-BMA250_Y_AXIS_LSB_LENGTH);
}

/** Get X-axis accelerometer reading.
 * @return X-axis acceleration measurement in 16-bit 2's complement format
 * @see BMA250_RA_X_AXIS_LSB
 */
int16_t BMA250::getAccelerationX() {
    I2Cdev::readBytes(devAddr, BMA250_RA_X_AXIS_LSB, 2, buffer);
    return ((((int16_t)buffer[1]) << 8) | buffer[0]) >> (8-BMA250_X_AXIS_LSB_LENGTH);
}

/** Get Y-axis accelerometer reading.
 * @return Y-axis acceleration measurement in 16-bit 2's complement format
 * @see BMA250_RA_Y_AXIS_LSB
 */
int16_t BMA250::getAccelerationY() {
    I2Cdev::readBytes(devAddr, BMA250_RA_Y_AXIS_LSB, 2, buffer);
    return ((((int16_t)buffer[1]) << 8) | buffer[0]) >> (8-BMA250_Y_AXIS_LSB_LENGTH);
}

/** Get Z-axis accelerometer reading.
 * @return Z-axis acceleration measurement in 16-bit 2's complement format
 * @see BMA250_RA_Z_AXIS_LSB
 */
int16_t BMA250::getAccelerationZ() {
    I2Cdev::readBytes(devAddr, BMA250_RA_Z_AXIS_LSB, 2, buffer);
    return ((((int16_t)buffer[1]) << 8) | buffer[0]) >> (8-BMA250_Z_AXIS_LSB_LENGTH);
}

/** Check for new X axis acceleration data.
 * @return New X-Axis Data Status
 * @see BMA250_RA_X_AXIS_LSB
 */
bool BMA250::newDataX() {
    I2Cdev::readBit(devAddr, BMA250_RA_X_AXIS_LSB, BMA250_X_NEW_DATA_BIT, buffer);
	return buffer[0];
}

/** Check for new Y axis acceleration data.
 * @return New Y-Axis Data Status
 * @see BMA250_RA_Y_AXIS_LSB
 */
bool BMA250::newDataY() {
    I2Cdev::readBit(devAddr, BMA250_RA_Y_AXIS_LSB, BMA250_Y_NEW_DATA_BIT, buffer);
	return buffer[0];
}

/** Check for new Z axis acceleration data.
 * @return New Z-Axis Data Status
 * @see BMA250_RA_Z_AXIS_LSB
 */
bool BMA250::newDataZ() {
    I2Cdev::readBit(devAddr, BMA250_RA_Z_AXIS_LSB, BMA250_Z_NEW_DATA_BIT, buffer);
	return buffer[0];
}
				
// TEMP register
/** Check for current temperature
 * @return Current Temperature in 0.5C increments from -30C at 00h
 * @see BMA250_RA_TEMP_RD
 */
int8_t BMA250::getTemperature() {
    I2Cdev::readByte(devAddr, BMA250_RA_TEMP_RD, buffer);
    return buffer[0];
}
	
// RANGE / BANDWIDTH registers

/** Get Sensor Full Range
 * @return Current Sensor Full Scale Range
 * 0 = +/- 2G
 * 1 = +/- 4G
 * 2 = +/- 8G
 * @see BMA250_RA_RANGE_BWIDTH
 * @see BMA250_RANGE_BIT
 * @see BMA250_RANGE_LENGTH
 */
uint8_t BMA250::getRange() {
    I2Cdev::readByte(devAddr, BMA250_RA_PMU_RANGE, buffer);
    return buffer[0];
}

/** Set Sensor Full Range
 * @param range New full-scale range value
 * @see getRange()
 * @see BMA250_RA_RANGE_BWIDTH
 * @see BMA250_RANGE_BIT
 * @see BMA250_RANGE_LENGTH
 */
void BMA250::setRange(uint8_t range) {
    I2Cdev::writeByte(devAddr, BMA250_RA_PMU_RANGE, range);
}


/** Get digital filter bandwidth.
 * The bandwidth parameter is used to setup the digital filtering of ADC output data to obtain
 * the desired bandwidth.
 * @return Current Sensor Bandwidth
 * 0 = 25Hz
 * 1 = 50Hz
 * 2 = 100Hz
 * 3 = 190Hz
 * 4 = 375Hz
 * 5 = 750Hz
 * 6 = 2500Hz
 * @see BMA250_RA_RANGE_BWIDTH
 * @see BMA250_RANGE_BIT
 * @see BMA250_RANGE_LENGTH
 */
uint8_t BMA250::getBandwidth() {
    I2Cdev::readByte(devAddr, BMA250_RA_PMU_BW, buffer);
    return buffer[0];
}

/** Set Sensor Full Range
 * @param bandwidth New bandwidth value
 * @see getBandwidth()
 * @see BMA250_RA_RANGE_BWIDTH
 * @see BMA250_RANGE_BIT
 * @see BMA250_RANGE_LENGTH
 */
void BMA250::setBandwidth(uint8_t bandwidth) {
    I2Cdev::writeByte(devAddr, BMA250_RA_PMU_BW, bandwidth);
}
