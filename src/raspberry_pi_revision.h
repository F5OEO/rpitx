//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2015 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#ifndef RASPBERRY_PI_INFO_H
#define RASPBERRY_PI_INFO_H

//-------------------------------------------------------------------------

#include <stdint.h>

//-------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------

#define RPI_PERIPHERAL_BASE_UNKNOWN 0
#define RPI_BROADCOM_2835_PERIPHERAL_BASE 0x20000000
#define RPI_BROADCOM_2836_PERIPHERAL_BASE 0x3F000000
#define RPI_BROADCOM_2837_PERIPHERAL_BASE 0x3F000000

typedef enum
{
    RPI_MEMORY_UNKNOWN = -1,
    RPI_256MB = 256,
    RPI_512MB = 512,
    RPI_1024MB = 1024,
}
RASPBERRY_PI_MEMORY_T;

typedef enum
{
    RPI_PROCESSOR_UNKNOWN = -1,
    RPI_BROADCOM_2835 = 2835,
    RPI_BROADCOM_2836 = 2836,
    RPI_BROADCOM_2837 = 2837
}
RASPBERRY_PI_PROCESSOR_T;

typedef enum
{
    RPI_I2C_DEVICE_UNKNOWN = -1,
    RPI_I2C_0 = 0,
    RPI_I2C_1 = 1
}
RASPBERRY_PI_I2C_DEVICE_T;

typedef enum
{
    RPI_MODEL_UNKNOWN = -1,
    RPI_MODEL_A,
    RPI_MODEL_B,
    RPI_MODEL_A_PLUS,
    RPI_MODEL_B_PLUS,
    RPI_MODEL_B_PI_2,
    RPI_MODEL_ALPHA,
    RPI_COMPUTE_MODULE,
    RPI_MODEL_ZERO,
    RPI_MODEL_B_PI_3
}
RASPBERRY_PI_MODEL_T;

typedef enum
{
    RPI_MANUFACTURER_UNKNOWN = -1,
    RPI_MANUFACTURER_SONY,
    RPI_MANUFACTURER_EGOMAN,
    RPI_MANUFACTURER_QISDA,
    RPI_MANUFACTURER_EMBEST,
}
RASPBERRY_PI_MANUFACTURER_T;

//-------------------------------------------------------------------------

typedef struct
{
    RASPBERRY_PI_MEMORY_T memory;
    RASPBERRY_PI_PROCESSOR_T processor;
    RASPBERRY_PI_I2C_DEVICE_T i2cDevice;
    RASPBERRY_PI_MODEL_T model;
    RASPBERRY_PI_MANUFACTURER_T manufacturer;
    int pcbRevision;
    int warrantyBit;
    int revisionNumber;
    uint32_t peripheralBase;
}
RASPBERRY_PI_INFO_T;

//-------------------------------------------------------------------------

// getRaspberryPiInformation()
//
// return - 0 - failed to get revision from /proc/cpuinfo
//          1 - found classic revision number
//          2 - found Pi 2 style revision number

int
getRaspberryPiInformation(
    RASPBERRY_PI_INFO_T *info);

int
getRaspberryPiInformationForRevision(
    int revision,
    RASPBERRY_PI_INFO_T *info);

int
getRaspberryPiRevision(void);

const char *
raspberryPiMemoryToString(
    RASPBERRY_PI_MEMORY_T memory);

const char *
raspberryPiProcessorToString(
    RASPBERRY_PI_PROCESSOR_T processor);

const char *
raspberryPiI2CDeviceToString(
    RASPBERRY_PI_I2C_DEVICE_T i2cDevice);

const char *
raspberryPiModelToString(
    RASPBERRY_PI_MODEL_T model);

const char *
raspberryPiManufacturerToString(
    RASPBERRY_PI_MANUFACTURER_T manufacturer);

//-------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//-------------------------------------------------------------------------

#endif
