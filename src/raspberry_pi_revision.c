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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raspberry_pi_revision.h"

//-------------------------------------------------------------------------
//
// The file /proc/cpuinfo contains a line such as:-
//
// Revision    : 0003
//
// that holds the revision number of the Raspberry Pi.
// Known revisions (prior to the Raspberry Pi 2) are:
//
//     +----------+---------+---------+--------+-------------+
//     | Revision |  Model  | PCB Rev | Memory | Manufacture |
//     +----------+---------+---------+--------+-------------+
//     |   0000   |         |         |        |             |
//     |   0001   |         |         |        |             |
//     |   0002   |    B    |    1    | 256 MB |             |
//     |   0003   |    B    |    1    | 256 MB |             |
//     |   0004   |    B    |    2    | 256 MB |   Sony      |
//     |   0005   |    B    |    2    | 256 MB |   Qisda     |
//     |   0006   |    B    |    2    | 256 MB |   Egoman    |
//     |   0007   |    A    |    2    | 256 MB |   Egoman    |
//     |   0008   |    A    |    2    | 256 MB |   Sony      |
//     |   0009   |    A    |    2    | 256 MB |   Qisda     |
//     |   000a   |         |         |        |             |
//     |   000b   |         |         |        |             |
//     |   000c   |         |         |        |             |
//     |   000d   |    B    |    2    | 512 MB |   Egoman    |
//     |   000e   |    B    |    2    | 512 MB |   Sony      |
//     |   000f   |    B    |    2    | 512 MB |   Qisda     |
//     |   0010   |    B+   |    1    | 512 MB |   Sony      |
//     |   0011   | compute |    1    | 512 MB |   Sony      |
//     |   0012   |    A+   |    1    | 256 MB |   Sony      |
//     |   0013   |    B+   |    1    | 512 MB |   Embest    |
//     |   0014   | compute |    1    | 512 MB |   Sony      |
//     |   0015   |    A+   |    1    | 256 MB |   Sony      |
//     +----------+---------+---------+--------+-------------+
//
// If the Raspberry Pi has been over-volted (voiding the warranty) the
// revision number will have 100 at the front. e.g. 1000002.
//
//-------------------------------------------------------------------------
//
// With the release of the Raspberry Pi 2, there is a new encoding of the
// Revision field in /proc/cpuinfo. The bit fields are as follows
//
//     +----+----+----+----+----+----+----+----+
//     |FEDC|BA98|7654|3210|FEDC|BA98|7654|3210|
//     +----+----+----+----+----+----+----+----+
//     |    |    |    |    |    |    |    |AAAA|
//     |    |    |    |    |    |BBBB|BBBB|    |
//     |    |    |    |    |CCCC|    |    |    |
//     |    |    |    |DDDD|    |    |    |    |
//     |    |    | EEE|    |    |    |    |    |
//     |    |    |F   |    |    |    |    |    |
//     |    |   G|    |    |    |    |    |    |
//     |    |  H |    |    |    |    |    |    |
//     +----+----+----+----+----+----+----+----+
//     |1098|7654|3210|9876|5432|1098|7654|3210|
//     +----+----+----+----+----+----+----+----+
//
// +---+-------+--------------+--------------------------------------------+
// | # | bits  |   contains   | values                                     |
// +---+-------+--------------+--------------------------------------------+
// | A | 00-03 | PCB Revision | (the pcb revision number)                  |
// | B | 04-11 | Model name   | A, B, A+, B+, B Pi2, Alpha, Compute Module |
// |   |       |              | unknown, unknown, Zero                     |
// | C | 12-15 | Processor    | BCM2835, BCM2836, BCM2837                  |
// | D | 16-19 | Manufacturer | Sony, Egoman, Embest, unknown, Embest      |
// | E | 20-22 | Memory size  | 256 MB, 512 MB, 1024 MB                    |
// | F | 23-23 | encoded flag | (if set, revision is a bit field)          |
// | G | 24-24 | waranty bit  | (if set, warranty void - Pre Pi2)          |
// | H | 25-25 | waranty bit  | (if set, warranty void - Post Pi2)         |
// +---+-------+--------------+--------------------------------------------+
//
// Also, due to some early issues the warranty bit has been move from bit
// 24 to bit 25 of the revision number (i.e. 0x2000000).
//
// e.g.
//
// Revision    : A01041
//
// A - PCB Revision - 1 (first revision)
// B - Model Name - 4 (Model B Pi 2)
// C - Processor - 1 (BCM2836)
// D - Manufacturer - 0 (Sony)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : A21041
//
// A - PCB Revision - 1 (first revision)
// B - Model Name - 4 (Model B Pi 2)
// C - Processor - 1 (BCM2836)
// D - Manufacturer - 2 (Embest)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : 900092
//
// A - PCB Revision - 2 (second revision)
// B - Model Name - 9 (Model Zero)
// C - Processor - 0 (BCM2835)
// D - Manufacturer - 0 (Sony)
// E - Memory - 1 (512 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
// Revision    : A02082
//
// A - PCB Revision - 2 (first revision)
// B - Model Name - 8 (Model B Pi 3)
// C - Processor - 2 (BCM2837)
// D - Manufacturer - 0 (Sony)
// E - Memory - 2 (1024 MB)
// F - Endcoded flag - 1 (encoded cpu info)
//
//-------------------------------------------------------------------------

static RASPBERRY_PI_MEMORY_T revisionToMemory[] =
{
    RPI_MEMORY_UNKNOWN, //  0
    RPI_MEMORY_UNKNOWN, //  1
    RPI_256MB,          //  2
    RPI_256MB,          //  3
    RPI_256MB,          //  4
    RPI_256MB,          //  5
    RPI_256MB,          //  6
    RPI_256MB,          //  7
    RPI_256MB,          //  8
    RPI_256MB,          //  9
    RPI_MEMORY_UNKNOWN, //  A
    RPI_MEMORY_UNKNOWN, //  B
    RPI_MEMORY_UNKNOWN, //  C
    RPI_512MB,          //  D
    RPI_512MB,          //  E
    RPI_512MB,          //  F
    RPI_512MB,          // 10
    RPI_512MB,          // 11
    RPI_256MB,          // 12
    RPI_512MB,          // 13
    RPI_512MB,          // 14
    RPI_256MB           // 15
};

static RASPBERRY_PI_MEMORY_T bitFieldToMemory[] =
{
    RPI_256MB,
    RPI_512MB,
    RPI_1024MB
};

//-------------------------------------------------------------------------

static RASPBERRY_PI_PROCESSOR_T bitFieldToProcessor[] =
{
    RPI_BROADCOM_2835,
    RPI_BROADCOM_2836,
    RPI_BROADCOM_2837
};

//-------------------------------------------------------------------------

static RASPBERRY_PI_I2C_DEVICE_T revisionToI2CDevice[] =
{
    RPI_I2C_DEVICE_UNKNOWN, //  0
    RPI_I2C_DEVICE_UNKNOWN, //  1
    RPI_I2C_0,              //  2
    RPI_I2C_0,              //  3
    RPI_I2C_1,              //  4
    RPI_I2C_1,              //  5
    RPI_I2C_1,              //  6
    RPI_I2C_1,              //  7
    RPI_I2C_1,              //  8
    RPI_I2C_1,              //  9
    RPI_I2C_DEVICE_UNKNOWN, //  A
    RPI_I2C_DEVICE_UNKNOWN, //  B
    RPI_I2C_DEVICE_UNKNOWN, //  C
    RPI_I2C_1,              //  D
    RPI_I2C_1,              //  E
    RPI_I2C_1,              //  F
    RPI_I2C_1,              // 10
    RPI_I2C_1,              // 11
    RPI_I2C_1,              // 12
    RPI_I2C_1,              // 13
    RPI_I2C_1,              // 14
    RPI_I2C_1               // 15
};

//-------------------------------------------------------------------------

static RASPBERRY_PI_MODEL_T bitFieldToModel[] =
{
    RPI_MODEL_A,
    RPI_MODEL_B,
    RPI_MODEL_A_PLUS,
    RPI_MODEL_B_PLUS,
    RPI_MODEL_B_PI_2,
    RPI_MODEL_ALPHA,
    RPI_COMPUTE_MODULE,
    RPI_MODEL_UNKNOWN,
    RPI_MODEL_B_PI_3,
    RPI_MODEL_ZERO
};

static RASPBERRY_PI_MODEL_T revisionToModel[] =
{
    RPI_MODEL_UNKNOWN,  //  0
    RPI_MODEL_UNKNOWN,  //  1
    RPI_MODEL_B,        //  2
    RPI_MODEL_B,        //  3
    RPI_MODEL_B,        //  4
    RPI_MODEL_B,        //  5
    RPI_MODEL_B,        //  6
    RPI_MODEL_A,        //  7
    RPI_MODEL_A,        //  8
    RPI_MODEL_A,        //  9
    RPI_MODEL_UNKNOWN,  //  A
    RPI_MODEL_UNKNOWN,  //  B
    RPI_MODEL_UNKNOWN,  //  C
    RPI_MODEL_B,        //  D
    RPI_MODEL_B,        //  E
    RPI_MODEL_B,        //  F
    RPI_MODEL_B_PLUS,   // 10
    RPI_COMPUTE_MODULE, // 11
    RPI_MODEL_A_PLUS,   // 12
    RPI_MODEL_B_PLUS,   // 13
    RPI_COMPUTE_MODULE, // 14
    RPI_MODEL_A_PLUS    // 15
};

//-------------------------------------------------------------------------

static RASPBERRY_PI_MANUFACTURER_T bitFieldToManufacturer[] =
{
    RPI_MANUFACTURER_SONY,
    RPI_MANUFACTURER_EGOMAN,
    RPI_MANUFACTURER_EMBEST,
    RPI_MANUFACTURER_UNKNOWN,
    RPI_MANUFACTURER_EMBEST
};

static RASPBERRY_PI_MANUFACTURER_T revisionToManufacturer[] =
{
    RPI_MANUFACTURER_UNKNOWN, //  0
    RPI_MANUFACTURER_UNKNOWN, //  1
    RPI_MANUFACTURER_UNKNOWN, //  2
    RPI_MANUFACTURER_UNKNOWN, //  3
    RPI_MANUFACTURER_SONY,    //  4
    RPI_MANUFACTURER_QISDA,   //  5
    RPI_MANUFACTURER_EGOMAN,  //  6
    RPI_MANUFACTURER_EGOMAN,  //  7
    RPI_MANUFACTURER_SONY,    //  8
    RPI_MANUFACTURER_QISDA,   //  9
    RPI_MANUFACTURER_UNKNOWN, //  A
    RPI_MANUFACTURER_UNKNOWN, //  B
    RPI_MANUFACTURER_UNKNOWN, //  C
    RPI_MANUFACTURER_EGOMAN,  //  D
    RPI_MANUFACTURER_SONY,    //  E
    RPI_MANUFACTURER_QISDA,   //  F
    RPI_MANUFACTURER_SONY,    // 10
    RPI_MANUFACTURER_SONY,    // 11
    RPI_MANUFACTURER_SONY,    // 12
    RPI_MANUFACTURER_EMBEST,  // 13
    RPI_MANUFACTURER_SONY,    // 14
    RPI_MANUFACTURER_SONY     // 15
};

//-------------------------------------------------------------------------

static int revisionToPcbRevision[] =
{
    0, //  0
    0, //  1
    1, //  2
    1, //  3
    2, //  4
    2, //  5
    2, //  6
    2, //  7
    2, //  8
    2, //  9
    0, //  A
    0, //  B
    0, //  C
    2, //  D
    2, //  E
    2, //  F
    1, // 10
    1, // 11
    1, // 12
    1, // 13
    1, // 14
    1  // 15
};

//-------------------------------------------------------------------------
//
// Remove leading and trailing whitespace from a string.

static char *
trimWhiteSpace(
    char *string)
{
    if (string == NULL)
    {
        return NULL;
    }

    while (isspace(*string))
    {
        string++;
    }

    if (*string == '\0')
    {
        return string;
    }

    char *end = string;

    while (*end)
    {
        ++end;
    }
    --end;

    while ((end > string) && isspace(*end))
    {
        end--;
    }

    *(end + 1) = 0;
    return string;
}

//-------------------------------------------------------------------------

int
getRaspberryPiRevision()
{
    int raspberryPiRevision = 0;

    FILE *fp = fopen("/proc/cpuinfo", "r");

    if (fp == NULL)
    {
        perror("/proc/cpuinfo");
        return raspberryPiRevision;
    }

    char entry[80];

    while (fgets(entry, sizeof(entry), fp) != NULL)
    {
        char* saveptr = NULL;

        char *key = trimWhiteSpace(strtok_r(entry, ":", &saveptr));
        char *value = trimWhiteSpace(strtok_r(NULL, ":", &saveptr));

        if (strcasecmp("Revision", key) == 0)
        {
            raspberryPiRevision = strtol(value, NULL, 16);
        }
    }

    fclose(fp);

    return raspberryPiRevision;
}

//-------------------------------------------------------------------------

int
getRaspberryPiInformation(
    RASPBERRY_PI_INFO_T *info)
{
    int revision = getRaspberryPiRevision();

    return getRaspberryPiInformationForRevision(revision, info);
}

//-------------------------------------------------------------------------

int
getRaspberryPiInformationForRevision(
    int revision,
    RASPBERRY_PI_INFO_T *info)
{
    int result = 0;

    if (info != NULL)
    {
        info->memory = RPI_MEMORY_UNKNOWN;
        info->processor = RPI_PROCESSOR_UNKNOWN;
        info->i2cDevice = RPI_I2C_DEVICE_UNKNOWN;
        info->model = RPI_MODEL_UNKNOWN;
        info->manufacturer = RPI_MANUFACTURER_UNKNOWN;
        info->pcbRevision = 0;
        info->warrantyBit = 0;
        info->revisionNumber = revision;
        info->peripheralBase = RPI_PERIPHERAL_BASE_UNKNOWN;

        if (revision != 0)
        {
            size_t maxOriginalRevision = (sizeof(revisionToModel) /
                                         sizeof(revisionToModel[0])) - 1;

            // remove warranty bit

            revision &= ~0x3000000;

            if (revision & 0x800000)
            {
                // Raspberry Pi2 style revision encoding

                result = 2;

                if (info->revisionNumber & 0x2000000)
                {
                    info->warrantyBit = 1;
                }

                int memoryIndex = (revision & 0x700000) >> 20;
                size_t knownMemoryValues = sizeof(bitFieldToMemory)
                                         / sizeof(bitFieldToMemory[0]);

                if (memoryIndex < knownMemoryValues)
                {
                    info->memory = bitFieldToMemory[memoryIndex];
                }
                else
                {
                    info->memory = RPI_MEMORY_UNKNOWN;
                }

                int processorIndex = (revision & 0xF000) >> 12;
                size_t knownProcessorValues = sizeof(bitFieldToProcessor)
                                            / sizeof(bitFieldToProcessor[0]);
                if (processorIndex < knownProcessorValues)
                {
                    info->processor = bitFieldToProcessor[processorIndex];
                }
                else
                {
                    info->processor = RPI_PROCESSOR_UNKNOWN;
                }

                // If some future firmware changes the Rev number of
                // older Raspberry Pis, then need to work out the i2c
                // device.

                info->i2cDevice = RPI_I2C_1;

                int modelIndex = (revision & 0xFF0) >> 4;
                size_t knownModelValues = sizeof(bitFieldToModel)
                                        / sizeof(bitFieldToModel[0]);

                if (modelIndex < knownModelValues)
                {
                    info->model = bitFieldToModel[modelIndex];
                }
                else
                {
                    info->model = RPI_MODEL_UNKNOWN;
                }

                int madeByIndex = (revision & 0xF0000) >> 16;
                size_t knownManufacturerValues = sizeof(bitFieldToManufacturer)
                                               / sizeof(bitFieldToManufacturer[0]);

                if (madeByIndex < knownManufacturerValues)
                {
                    info->manufacturer = bitFieldToManufacturer[madeByIndex];
                }
                else
                {
                    info->manufacturer = RPI_MANUFACTURER_UNKNOWN;
                }

                info->pcbRevision = revision & 0xF;
            }
            else if (revision <= maxOriginalRevision)
            {
                // Original revision encoding

                result = 1;

                if (info->revisionNumber & 0x1000000)
                {
                    info->warrantyBit = 1;
                }

                info->memory = revisionToMemory[revision];
                info->i2cDevice = revisionToI2CDevice[revision];
                info->model = revisionToModel[revision];
                info->manufacturer = revisionToManufacturer[revision];
                info->pcbRevision = revisionToPcbRevision[revision];

                if (info->model == RPI_MODEL_UNKNOWN)
                {
                    info->processor = RPI_PROCESSOR_UNKNOWN;
                }
                else
                {
                    info->processor = RPI_BROADCOM_2835;
                }
            }
        }

        switch (info->processor)
        {
        case RPI_PROCESSOR_UNKNOWN:

            info->peripheralBase = RPI_PERIPHERAL_BASE_UNKNOWN;
            break;

        case RPI_BROADCOM_2835:

            info->peripheralBase = RPI_BROADCOM_2835_PERIPHERAL_BASE;
            break;

        case RPI_BROADCOM_2836:

            info->peripheralBase = RPI_BROADCOM_2836_PERIPHERAL_BASE;
            break;

        case RPI_BROADCOM_2837:

            info->peripheralBase = RPI_BROADCOM_2837_PERIPHERAL_BASE;
            break;
        }
    }

    return result;
}

//-------------------------------------------------------------------------

const char *
raspberryPiMemoryToString(
    RASPBERRY_PI_MEMORY_T memory)
{
    const char *string = "unknown";

    switch(memory)
    {
    case RPI_256MB:

        string = "256 MB";
        break;

    case RPI_512MB:

        string = "512 MB";
        break;

    case RPI_1024MB:

        string = "1024 MB";
        break;

    default:

        break;
    }

    return string;
}

//-------------------------------------------------------------------------

const char *
raspberryPiProcessorToString(
    RASPBERRY_PI_PROCESSOR_T processor)
{
    const char *string = "unknown";

    switch(processor)
    {
    case RPI_BROADCOM_2835:

        string = "Broadcom BCM2835";
        break;

    case RPI_BROADCOM_2836:

        string = "Broadcom BCM2836";
        break;

    case RPI_BROADCOM_2837:

        string = "Broadcom BCM2837";
        break;

    default:

        break;
    }

    return string;
}

//-------------------------------------------------------------------------

const char *
raspberryPiI2CDeviceToString(
    RASPBERRY_PI_I2C_DEVICE_T i2cDevice)
{
    const char *string = "unknown";

    switch(i2cDevice)
    {
    case RPI_I2C_0:

        string = "/dev/i2c-0";
        break;

    case RPI_I2C_1:

        string = "/dev/i2c-1";
        break;

    default:

        break;
    }

    return string;
}

//-------------------------------------------------------------------------

const char *
raspberryPiModelToString(
    RASPBERRY_PI_MODEL_T model)
{
    const char *string = "unknown";

    switch(model)
    {
    case RPI_MODEL_A:

        string = "Model A";
        break;

    case RPI_MODEL_B:

        string = "Model B";
        break;

    case RPI_MODEL_A_PLUS:

        string = "Model A+";
        break;

    case RPI_MODEL_B_PLUS:

        string = "Model B+";
        break;

    case RPI_MODEL_B_PI_2:

        string = "Model B Pi 2";
        break;

    case RPI_MODEL_ALPHA:

        string = "Alpha";
        break;

    case RPI_COMPUTE_MODULE:

        string = "Compute Module";
        break;

    case RPI_MODEL_ZERO:

        string = "Model Zero";
        break;

    case RPI_MODEL_B_PI_3:

        string = "Model B Pi 3";
        break;

    default:

        break;
    }

    return string;
}

//-------------------------------------------------------------------------

const char *
raspberryPiManufacturerToString(
    RASPBERRY_PI_MANUFACTURER_T manufacturer)
{
    const char *string = "unknown";

    switch(manufacturer)
    {
    case RPI_MANUFACTURER_SONY:

        string = "Sony";
        break;

    case RPI_MANUFACTURER_EGOMAN:

        string = "Egoman";
        break;

    case RPI_MANUFACTURER_QISDA:

        string = "Qisda";
        break;

    case RPI_MANUFACTURER_EMBEST:

        string = "Embest";
        break;

    default:

        break;
    }

    return string;
}

