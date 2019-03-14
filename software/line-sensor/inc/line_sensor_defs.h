#pragma once

#define NUMBER_OF_SENSORS 25

#define LSC_RESET                          0x01
#define LSC_START_CALIBRATION              0x02
#define LSC_FINISH_CALIBRATION             0x03
#define LSC_USE_CALIBRATION_DATA           0x04
#define LSC_USE_HARDCODED_CALIBRATION_DATA 0x05
#define LSC_SEND_SENSOR_DATA               0x06

#define LSS_NOT_READY               0x01
#define LSS_CRITICAL_FAILURE        0x02
#define LSS_CALIBRATION_ERROR       0x03
#define LSS_SENSOR_NOT_CALIBRATED   0x04

#define LSS_FLAG_NEW_DATA_AVAILABLE 0x80

// uncomment one of the following (based on which resistors are installed on the board,
// see schematics variants for more information)
// #define ACTIVE_COMMUNICATION_I2C
// #define ACTIVE_COMMUNICATION_SPI
#define ACTIVE_COMMUNICATION_USART
