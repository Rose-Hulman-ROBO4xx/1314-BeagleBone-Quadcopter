/**
 * MPU6050 minimal device I2C library for Node.js
 * Based on Jeff Rowberg's MPU6050 I2C device library.
 * 2012/05/26 by Jason Stapels <jstapels@gmail.com>
 * 
 * Changelog:
 *     XX - ToDo...
 */
//============================================================================================
// MPU6050 device I2C library code for Node.js is placed under the MIT license
// Copyright (c) 2013 Jason Stapels
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//================================================================================================

var i2c = require('i2c');

/**
 * Default constructor, uses default I2C address or default SS Pin if SPI
 * @see MPU6050.DEFAULT_ADDRESS
 */
function MPU6050(device, address) {
  this.device = device || '/dev/i2c-1';
  this.address = address || MPU6050.DEFAULT_ADDRESS;
}

MPU6050.ADDRESS_AD0_LOW = 0x68; // address pin low (GND); default for InvenSense evaluation board
MPU6050.ADDRESS_AD0_HIGH = 0x69; // address pin high (VCC)
MPU6050.DEFAULT_ADDRESS = MPU6050.ADDRESS_AD0_LOW;

/**
 * Power on and prepare for general usage.
 * This will activate the device and take it out of sleep mode (which must be done
 * after start-up). This function also sets both the accelerometer and the gyroscope
 * to their most sensitive settings, namely +/- 2g and +/- 250 degrees/sec, and sets
 * the clock source to use the X Gyro for reference, which is slightly better than
 * the default internal clock source.
 */
MPU6050.prototype.initialize = function() {
  this.i2cdev = new I2cDev(this.address, {device : this.device});
  
  this.setClockSource(MPU6050.CLOCK_PLL_XGYRO);
  this.setFullScaleGyroRange(MPU6050.GYRO_FS_250);
  this.setFullScaleAccelRange(MPU6050.ACCEL_FS_2);
  this.setSleepEnabled(false);
};

/**
 * Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
MPU6050.prototype.testConnection = function() {
  return this.getDeviceID() === 0x34;
};

// WHO_AM_I register

MPU6050.RA_WHO_AM_I = 0x75;
MPU6050.WHO_AM_I_BIT = 6;
MPU6050.WHO_AM_I_LENGTH = 6;

/**
 * Get Device ID.
 * This register is used to verify the identity of the device (0b110100).
 * @return Device ID (should be 0x68, 104 dec, 150 oct)
 */
MPU6050.prototype.getDeviceID = function() {
  return this.i2cdev.readBits(MPU6050.RA_WHO_AM_I, MPU6050.WHO_AM_I_BIT, MPU6050.WHO_AM_I_LENGTH);
};

/**
 * Set Device ID.
 * Write a new ID into the WHO_AM_I register (no idea why this should ever be
 * necessary though).
 * @param id New device ID to set.
 * @see getDeviceID()
 */
MPU6050.prototype.setDeviceID = function(id) {
  this.i2cdev.writeBits(MPU6050.RA_WHO_AM_I, MPU6050.WHO_AM_I_BIT, MPU6050.WHO_AM_I_LENGTH, id);
};

// GYRO_CONFIG register

MPU6050.RA_GYRO_CONFIG = 0x1B;
MPU6050.GCONFIG_FS_SEL_BIT = 4;
MPU6050.GCONFIG_FS_SEL_LENGTH = 2;
MPU6050.GYRO_FS_250  = 0x00;
MPU6050.GYRO_FS_500  = 0x01;
MPU6050.GYRO_FS_1000 = 0x02;
MPU6050.GYRO_FS_2000 = 0x03;

/**
 * Get full-scale gyroscope range.
 * The FS_SEL parameter allows setting the full-scale range of the gyro sensors,
 * as described in the table below.
 *
 * <pre>
 * 0 = +/- 250 degrees/sec
 * 1 = +/- 500 degrees/sec
 * 2 = +/- 1000 degrees/sec
 * 3 = +/- 2000 degrees/sec
 * </pre>
 *
 * @return Current full-scale gyroscope range setting
 */
MPU6050.prototype.getFullScaleGyroRange = function() {
  return this.i2cdev.readBits(MPU6050.RA_GYRO_CONFIG, MPU6050.GCONFIG_FS_SEL_BIT, MPU6050.GCONFIG_FS_SEL_LENGTH);
};

/**
 * Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
MPU6050.prototype.setFullScaleGyroRange = function(range) {
  this.i2cdev.writeBits(MPU6050.RA_GYRO_CONFIG, MPU6050.GCONFIG_FS_SEL_BIT, MPU6050.GCONFIG_FS_SEL_LENGTH, range);
};

// ACCEL_CONFIG register

MPU6050.RA_ACCEL_CONFIG = 0x1C;
MPU6050.ACONFIG_AFS_SEL_BIT = 4;
MPU6050.ACONFIG_AFS_SEL_LENGTH = 2;
MPU6050.ACCEL_FS_2  = 0x00;
MPU6050.ACCEL_FS_4  = 0x01;
MPU6050.ACCEL_FS_8  = 0x02;
MPU6050.ACCEL_FS_16 = 0x03;

/**
 * Get full-scale accelerometer range.
 * The FS_SEL parameter allows setting the full-scale range of the accelerometer
 * sensors, as described in the table below.
 *
 * <pre>
 * 0 = +/- 2g
 * 1 = +/- 4g
 * 2 = +/- 8g
 * 3 = +/- 16g
 * </pre>
 *
 * @return Current full-scale accelerometer range setting
 */
MPU6050.prototype.getFullScaleAccelRange = function() {
  return this.i2cdev.readBits(MPU6050.RA_ACCEL_CONFIG, MPU6050.ACONFIG_AFS_SEL_BIT, MPU6050.ACONFIG_AFS_SEL_LENGTH);
};

/**
 * Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 */
MPU6050.prototype.setFullScaleAccelRange = function(range) {
  this.i2cdev.writeBits(MPU6050.RA_ACCEL_CONFIG, MPU6050.ACONFIG_AFS_SEL_BIT, MPU6050.ACONFIG_AFS_SEL_LENGTH, range);
};

// ACCEL_*OUT_* registers

MPU6050.RA_ACCEL_XOUT_H = 0x3B;
MPU6050.RA_ACCEL_XOUT_L = 0x3C;
MPU6050.RA_ACCEL_YOUT_H = 0x3D;
MPU6050.RA_ACCEL_YOUT_L = 0x3E;
MPU6050.RA_ACCEL_ZOUT_H = 0x3F;
MPU6050.RA_ACCEL_ZOUT_L = 0x40;

/**
 * Get 3-axis accelerometer readings.
 * These registers store the most recent accelerometer measurements.
 * Accelerometer measurements are written to these registers at the Sample Rate
 * as defined in Register 25.
 *
 * The accelerometer measurement registers, along with the temperature
 * measurement registers, gyroscope measurement registers, and external sensor
 * data registers, are composed of two sets of registers: an internal register
 * set and a user-facing read register set.
 *
 * The data within the accelerometer sensors' internal register set is always
 * updated at the Sample Rate. Meanwhile, the user-facing read register set
 * duplicates the internal register set's data values whenever the serial
 * interface is idle. This guarantees that a burst read of sensor registers will
 * read measurements from the same sampling instant. Note that if burst reads
 * are not used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Each 16-bit accelerometer measurement has a full scale defined in ACCEL_FS
 * (Register 28). For each full scale setting, the accelerometers' sensitivity
 * per LSB in ACCEL_xOUT is shown in the table below:
 *
 * <pre>
 * AFS_SEL | Full Scale Range | LSB Sensitivity
 * --------+------------------+----------------
 * 0       | +/- 2g           | 8192 LSB/mg
 * 1       | +/- 4g           | 4096 LSB/mg
 * 2       | +/- 8g           | 2048 LSB/mg
 * 3       | +/- 16g          | 1024 LSB/mg
 * </pre>
 * 
 * @return An array containing the three accellerations.
 */
MPU6050.prototype.getAcceleration = function() {
  buffer = this.i2cdev.readBytes(MPU6050.RA_ACCEL_XOUT_H, 6);
  return [
    buffer.readInt16BE(0),
    buffer.readInt16BE(2),
    buffer.readInt16BE(4)
  ];
};

/**
 * Get raw 6-axis motion sensor readings (accel/gyro).
 * Retrieves all currently available motion sensor values.
 * @see getAcceleration()
 * @see getRotation()
 */
MPU6050.prototype.getMotion6 = function() {
  buffer = this.i2cdev.readBytes(MPU6050.RA_ACCEL_XOUT_H, 14);
  
  return [
      buffer.readInt16BE(0),
      buffer.readInt16BE(2),
      buffer.readInt16BE(4),
      buffer.readInt16BE(8),
      buffer.readInt16BE(10),
      buffer.readInt16BE(12)
  ];
};

// GYRO_*OUT_* registers

MPU6050.RA_GYRO_XOUT_H = 0x43;
MPU6050.RA_GYRO_XOUT_L = 0x44;
MPU6050.RA_GYRO_YOUT_H = 0x45;
MPU6050.RA_GYRO_YOUT_L = 0x46;
MPU6050.RA_GYRO_ZOUT_H = 0x47;
MPU6050.RA_GYRO_ZOUT_L = 0x48;

/**
 * Get 3-axis gyroscope readings.
 * These gyroscope measurement registers, along with the accelerometer
 * measurement registers, temperature measurement registers, and external sensor
 * data registers, are composed of two sets of registers: an internal register
 * set and a user-facing read register set.
 * The data within the gyroscope sensors' internal register set is always
 * updated at the Sample Rate. Meanwhile, the user-facing read register set
 * duplicates the internal register set's data values whenever the serial
 * interface is idle. This guarantees that a burst read of sensor registers will
 * read measurements from the same sampling instant. Note that if burst reads
 * are not used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Each 16-bit gyroscope measurement has a full scale defined in FS_SEL
 * (Register 27). For each full scale setting, the gyroscopes' sensitivity per
 * LSB in GYRO_xOUT is shown in the table below:
 *
 * <pre>
 * FS_SEL | Full Scale Range   | LSB Sensitivity
 * -------+--------------------+----------------
 * 0      | +/- 250 degrees/s  | 131 LSB/deg/s
 * 1      | +/- 500 degrees/s  | 65.5 LSB/deg/s
 * 2      | +/- 1000 degrees/s | 32.8 LSB/deg/s
 * 3      | +/- 2000 degrees/s | 16.4 LSB/deg/s
 * </pre>
 *
 * @param x 16-bit signed integer container for X-axis rotation
 * @param y 16-bit signed integer container for Y-axis rotation
 * @param z 16-bit signed integer container for Z-axis rotation
 * @see getMotion6()
 */
MPU6050.prototype.getRotation = function() {
   var buffer = this.i2cdev.readBytes(MPU6050.RA_GYRO_XOUT_H, 6);
   return [buffer.readInt16BE(0), buffer.readInt16BE(2), buffer.readInt16BE(4)];  
};


// PWR_MGMT_1 register

MPU6050.RA_PWR_MGMT_1 = 0x6B;
MPU6050.PWR1_DEVICE_RESET_BIT = 7;
MPU6050.PWR1_SLEEP_BIT = 6;
MPU6050.PWR1_CYCLE_BIT = 5;
MPU6050.PWR1_TEMP_DIS_BIT = 3;
MPU6050.PWR1_CLKSEL_BIT = 2;
MPU6050.PWR1_CLKSEL_LENGTH = 3;

/** Get sleep mode status.
 * Setting the SLEEP bit in the register puts the device into very low power
 * sleep mode. In this mode, only the serial interface and internal registers
 * remain active, allowing for a very low standby current. Clearing this bit
 * puts the device back into normal mode. To save power, the individual standby
 * selections for each of the gyros should be used if any gyro axis is not used
 * by the application.
 * @return Current sleep mode enabled status
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
MPU6050.prototype.getSleepEnabled = function() {
  return this.i2cdev.readBit(MPU6050.RA_PWR_MGMT_1, MPU6050.PWR1_SLEEP_BIT);
};

/** Set sleep mode status.
 * @param enabled New sleep mode enabled status
 * @see getSleepEnabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
MPU6050.prototype.setSleepEnabled = function(enabled) {
  this.i2cdev.writeBit(MPU6050.RA_PWR_MGMT_1, MPU6050.PWR1_SLEEP_BIT, enabled);
};

/**
 * Get clock source setting.
 * @return Current clock source setting
 */
MPU6050.prototype.getClockSource = function() {
  return this.i2cdev.readBits(MPU6050.RA_PWR_MGMT_1, MPU6050.PWR1_CLKSEL_BIT, MPU6050.PWR1_CLKSEL_LENGTH);
};

/**
 * Set clock source setting.
 * An internal 8MHz oscillator, gyroscope based clock, or external sources can
 * be selected as the MPU-60X0 clock source. When the internal 8 MHz oscillator
 * or an external source is chosen as the clock source, the MPU-60X0 can operate
 * in low power modes with the gyroscopes disabled.
 *
 * Upon power up, the MPU-60X0 clock source defaults to the internal oscillator.
 * However, it is highly recommended that the device be configured to use one of
 * the gyroscopes (or an external clock source) as the clock reference for
 * improved stability. The clock source can be selected according to the following table:
 *
 * <pre>
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillator
 * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
 * </pre>
 *
 * @param source New clock source setting
 * @see getClockSource()
 */
MPU6050.prototype.setClockSource = function(source) {
  this.i2cdev.writeBits(MPU6050.RA_PWR_MGMT_1, MPU6050.PWR1_CLKSEL_BIT, MPU6050.PWR1_CLKSEL_LENGTH, source);
};

module.exports = MPU6050;

/**
 * This class extends the i2c library with some extra functionality available
 * in the i2cdev library that the MPU60X0 library uses.
 */
function I2cDev(address, options) {
  i2c.call(this, address, options);
}

I2cDev.prototype = Object.create(i2c.prototype);
I2cDev.prototype.constructor = I2cDev;

I2cDev.prototype.bitMask = function(bit, bitLength) {
  return ((1 << bitLength) - 1) << (1 + bit - bitLength);
};

I2cDev.prototype.readBits = function(func, bit, bitLength, callback) {
  var mask = this.bitMask(bit, bitLength);
  
  if (callback) {
    this.readBytes(func, 1, function(err, buf) {
      var bits = (buf[0] & mask) >> (1 + bit - bitLength);
      callback(err, bits);
    });
  } else {
    var buf = this.readBytes(func, 1);
    return (buf[0] & mask) >> (1 + bit - bitLength);
  }
};

I2cDev.prototype.readBit = function(func, bit, bitLength, callback) {
  return this.readBits(func, bit, 1, callback);
};

I2cDev.prototype.writeBits = function(func, bit, bitLength, value, callback) {
  var oldValue = this.readBytes(func, 1);
  var mask = this.bitMask(bit, bitLength);
  var newValue = oldValue ^ ((oldValue ^ (value << bit)) & mask);
  this.writeBytes(func, [newValue], callback);
};

I2cDev.prototype.writeBit = function(func, bit, value, callback) {
  this.writeBits(func, bit, 1, value, callback);
};

