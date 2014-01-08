# MPU6050

Minimal Node.js module for reading the raw data from an MPU6050.

This module has been tested on a BeagleBone Black.

## Install

```
npm install mpu6050
```

## Usage

```javascript
var mpu6050 = require('mpu6050');

// Instantiate and initialize.
var mpu = new mpu6050();
mpu.initialize();

// Test the connection before using.
if (mpu.testConnection()) {
  console.log(mpu.getMotion6());
}

// Put the MPU6050 back to sleep.
mpu.setSleepEnabled(1);
```

