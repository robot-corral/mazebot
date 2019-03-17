#pragma once

// uncomment one of the following (based on which resistors are installed on the board,
// see schematics variants for more information)
// #define ACTIVE_COMMUNICATION_I2C
// #define ACTIVE_COMMUNICATION_SPI
#define ACTIVE_COMMUNICATION_USART

#define NUMBER_OF_SENSORS 25

#define USART_BAUDRATE 460800

#define COMMAND_PREFIX          0b0101010110101010
#define COMMAND_RESPONSE_PREFIX 0b1010101001010101

/*******************************************************************************
 * Line sensor commands
 ******************************************************************************/

#define LSC_RESET                          0x01
#define LSC_START_CALIBRATION              0x02
#define LSC_FINISH_CALIBRATION             0x03
#define LSC_USE_CALIBRATION_DATA           0x04
#define LSC_USE_HARDCODED_CALIBRATION_DATA 0x05
#define LSC_SEND_SENSOR_DATA               0x06

/*******************************************************************************
 * Line sensor status
 ******************************************************************************/

#define LSS_OK                               0x0000

#define LSS_OK_FLAG_NEW_DATA_AVAILABLE       0x0001

#define LSS_ERR_FLAG_NOT_READY               0x0002
#define LSS_ERR_FLAG_CALIBRATION_ERROR       0x0002

#define LSS_ERR_FLAG_SENSOR_NOT_CALIBRATED   0x0004

#define LSS_ERR_FLAG_TRANSMISSION_ERROR      0x0008
#define LSS_ERR_FLAG_WATCHDOG_RESET_DETECTED 0x0010
#define LSS_ERR_FLAG_ADC_DMA_FAILURE         0x0020
#define LSS_ERR_FLAG_DATA_BUFFER_CORRUPTED   0x0040

// only used when USART is an active communication device

#define LSS_ERR_FLAG_USART_DMA_FAILURE       0x0080
#define LSS_ERR_FLAG_USART_NOISE_ERROR       0x0100
#define LSS_ERR_FLAG_USART_FRAMING_ERROR     0x0200
#define LSS_ERR_FLAG_USART_OVERRUN_ERROR     0x0400
