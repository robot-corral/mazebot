#pragma once

#include <stdint.h>

/*******************************************************************************
 * Hardware revision: line-sensor-var1a-r3-2019-10-06
 ******************************************************************************/

#define NUMBER_OF_SENSORS ((uint8_t) 25)

#define CRC_POLYNOMIAL    ((uint16_t) 0xE5CC)

/*******************************************************************************
 * Line sensor commands (lineSensorCommandCode_t)
 ******************************************************************************/

#define LSC_NONE                           ((uint8_t) 0x00)
#define LSC_GET_SENSOR_VALUES              ((uint8_t) 0x01)
#define LSC_START_PHYSICAL_CALIBRATION     ((uint8_t) 0x02)
#define LSC_GET_CALIBRATION_STATUS         ((uint8_t) 0x03)
#define LSC_FINISH_CALIBRATION             ((uint8_t) 0x04)
#define LSC_GET_CALIBRATION_VALUES         ((uint8_t) 0x05)
#define LSC_GET_DETAILED_SENSOR_STATUS     ((uint8_t) 0x06)
/*
 * reset command restarts linse sensor and next command after reset will be default command
 * response LSC_GET_SENSOR_VALUES (so this command doesn't have response of its own).
 */
#define LSC_RESET                          ((uint8_t) 0x07)

#define LSC_FIRST_COMMAND LSC_GET_SENSOR_VALUES
#define LSC_LAST_COMMAND  LSC_RESET

/*******************************************************************************
 * Line sensor status (lineSensorStatus_t)
 ******************************************************************************/

#define LSS_OK                                 ((uint8_t) 0x00)
#define LSS_OK_FLAG_NEW_DATA_AVAILABLE         ((uint8_t) 0x01)
#define LSS_ERROR                              ((uint8_t) 0x02)

/*******************************************************************************
 * Line sensor detailed status (lineSensorDetailedStatus_t)
 ******************************************************************************/

#define LSDS_OK                                 ((uint32_t) 0x00000000)

#define LSDS_OK_FLAG_NEW_DATA_AVAILABLE         ((uint32_t) 0x00000001)

#define LSDS_ERR_FLAG_UNKNOWN_COMMAND_REQUESTED ((uint32_t) 0x00000002)

#define LSDS_ERR_FLAG_WATCHDOG_RESET_DETECTED   ((uint32_t) 0x00000004)

#define LSDS_ERR_FLAG_ADC_DMA_FAILURE           ((uint32_t) 0x00000008)
#define LSDS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED ((uint32_t) 0x00000010)

#define LSDS_ERR_FLAG_SPI_RX_ERROR              ((uint32_t) 0x00000020)
#define LSDS_ERR_FLAG_SPI_DMA_FAILURE           ((uint32_t) 0x00000040)
#define LSDS_ERR_FLAG_SPI_ERROR_CRC             ((uint32_t) 0x00000080)
#define LSDS_ERR_FLAG_SPI_ERROR_MODE_FAULT      ((uint32_t) 0x00000100)
#define LSDS_ERR_FLAG_SPI_ERROR_OVERRUN         ((uint32_t) 0x00000200)
#define LSDS_ERR_FLAG_SPI_ERROR_FRAME_FORMAT    ((uint32_t) 0x00000400)
#define LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE ((uint32_t) 0x00000800)

#define LSDS_ERR_FLAG_ALL                       ((uint32_t) 0xFFFFFFFE)

#define LSDS_ERR_FLAG_ADC_ALL                   ((LSDS_ERR_FLAG_ADC_DMA_FAILURE) | (LSDS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED))
