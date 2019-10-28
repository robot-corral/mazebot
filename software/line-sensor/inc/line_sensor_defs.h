#pragma once

/*******************************************************************************
 * Hardware revision: line-sensor-var1a-r3-2019-10-06
 ******************************************************************************/

#define NUMBER_OF_SENSORS 25

#define COMMAND_PREFIX          0b01010101
#define COMMAND_RESPONSE_PREFIX 0b10101010

#define SPI_CRC_POLYNOMIAL      0x9EB2

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

#define LSS_ZERO                               0x0000

#define LSS_OK_FLAG_NEW_DATA_AVAILABLE         0x0001

#define LSS_ERR_FLAG_NOT_READY                 0x0002
#define LSS_ERR_FLAG_CALIBRATION_ERROR         0x0004
#define LSS_ERR_FLAG_SENSOR_NOT_CALIBRATED     0x0008
#define LSS_ERR_FLAG_UNKNOWN_COMMAND           0x0010

#define LSS_ERR_FLAG_WATCHDOG_RESET_DETECTED   0x0020

#define LSS_ERR_FLAG_ADC_DMA_FAILURE           0x0040
#define LSS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED 0x0080

#define LSS_ERR_FLAG_SPI_RECEIVING_ERROR       0x0100
#define LSS_ERR_FLAG_SPI_DMA_FAILURE           0x0200
#define LSS_ERR_FLAG_SPI_ERROR_CRC             0x0400
#define LSS_ERR_FLAG_SPI_ERROR_MODE_FAULT      0x0800
#define LSS_ERR_FLAG_SPI_ERROR_OVERRUN         0x1000
#define LSS_ERR_FLAG_SPI_ERROR_FRAME_FORMAT    0x2000

#define LSS_ERR_FLAG_ADC_ALL                   ((LSS_ERR_FLAG_ADC_DMA_FAILURE) | (LSS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED))
