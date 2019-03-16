#pragma once

#define NUMBER_OF_SENSORS 25

#define USART_BAUDRATE 1152000

// Line sensor commands

#define LSC_RESET                          0x01
#define LSC_START_CALIBRATION              0x02
#define LSC_FINISH_CALIBRATION             0x03
#define LSC_USE_CALIBRATION_DATA           0x04
#define LSC_USE_HARDCODED_CALIBRATION_DATA 0x05
#define LSC_SEND_SENSOR_DATA               0x06

// Line sensor status

#define LSS_OK                          0x00

#define LSS_ERR_NOT_READY               0x01
#define LSS_ERR_CALIBRATION_ERROR       0x02
#define LSS_ERR_SENSOR_NOT_CALIBRATED   0x02

#define LSS_ERR_WATCHDOG_RESET_DETECTED 0x04
#define LSS_ERR_ADC_FAILURE             0x08
#define LSS_ERR_DATA_BUFFER_CORRUPTED   0x10

#define LSS_FLAG_NEW_DATA_AVAILABLE     0x80

// uncomment one of the following (based on which resistors are installed on the board,
// see schematics variants for more information)
// #define ACTIVE_COMMUNICATION_I2C
// #define ACTIVE_COMMUNICATION_SPI
#define ACTIVE_COMMUNICATION_USART
