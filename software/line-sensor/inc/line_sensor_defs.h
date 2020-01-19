/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>

/*******************************************************************************
 * Hardware revision: line-sensor-var1a-r3-2019-10-06
 ******************************************************************************/

#define LINE_SENSOR_NUMBER_OF_SENSORS ((uint8_t) 25)

#define LINE_SENSOR_CRC_POLYNOMIAL ((uint32_t) 0x4C11DB7)

/*******************************************************************************
 * Line sensor commands (lineSensorCommandCode_t)
 ******************************************************************************/

#define LSC_NONE                           ((uint8_t) 0x00)
#define LSC_GET_SENSOR_VALUES              ((uint8_t) 0x01)
#define LSC_START_CALIBRATION              ((uint8_t) 0x02)
/*
 * If no calibration was done after reset this will return all zeros.
 * This command can be called before, during calibration and after calibration is finished.
 */
#define LSC_GET_CALIBRATION_VALUES         ((uint8_t) 0x03)
#define LSC_FINISH_CALIBRATION             ((uint8_t) 0x04)
#define LSC_GET_DETAILED_SENSOR_STATUS     ((uint8_t) 0x05)
/*
 * reset command restarts linse sensor and next command after reset will be default command
 * response LSC_GET_SENSOR_VALUES (so this command doesn't have response of its own).
 */
#define LSC_RESET                          ((uint8_t) 0x06)

#define LSC_FIRST_COMMAND LSC_GET_SENSOR_VALUES
#define LSC_LAST_COMMAND  LSC_RESET

/*******************************************************************************
 * Line sensor status (lineSensorStatus_t)
 ******************************************************************************/

#define LSS_ZERO                       ((uint8_t) 0x00)
#define LSS_OK_FLAG_DATA_AVAILABLE     ((uint8_t) 0x01)
#define LSS_OK_FLAG_NEW_DATA_AVAILABLE ((uint8_t) 0x02)
#define LSS_ERROR                      ((uint8_t) 0x04)

/*******************************************************************************
 * Line sensor detailed status (lineSensorDetailedStatus_t)
 ******************************************************************************/

#define LSDS_OK                                              ((uint32_t) 0x00000000)
                                                             
#define LSDS_ERR_FLAG_WATCHDOG_RESET_DETECTED                ((uint32_t) 0x00000001)
                                                             
#define LSDS_ERR_FLAG_ADC_DMA_FAILURE                        ((uint32_t) 0x00000002)
#define LSDS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED              ((uint32_t) 0x00000004)
                                                             
#define LSDS_ERR_FLAG_SPI_RX_ERROR                           ((uint32_t) 0x00000008)
#define LSDS_ERR_FLAG_SPI_DMA_FAILURE                        ((uint32_t) 0x00000010)
#define LSDS_ERR_FLAG_SPI_ERROR_MODE_FAULT                   ((uint32_t) 0x00000020)
#define LSDS_ERR_FLAG_SPI_ERROR_OVERRUN                      ((uint32_t) 0x00000040)
#define LSDS_ERR_FLAG_SPI_ERROR_FRAME_FORMAT                 ((uint32_t) 0x00000080)
#define LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE              ((uint32_t) 0x00000100)
#define LSDS_ERR_FLAG_SPI_UNKNOWN_COMMAND_REQUESTED          ((uint32_t) 0x00000200)
#define LSDS_ERR_FLAG_SPI_INVALID_COMMAND_PARAMETER          ((uint32_t) 0x00000400)
#define LSDS_ERR_FLAG_SPI_NOT_ENOUGH_TIME_TO_PROCESS_COMMAND ((uint32_t) 0x00000800)
#define LSDS_ERR_FLAG_SPI_FAILURE_WHILE_TRANSMITTING_FILLER  ((uint32_t) 0x00001000)
#define LSDS_ERR_FLAG_SPI_TASK_SCHEDULER_IS_BUSY             ((uint32_t) 0x00002000)

#define LSDS_ERR_FLAG_ADC_ALL                       ((LSDS_ERR_FLAG_ADC_DMA_FAILURE) | \
                                                     (LSDS_ERR_FLAG_ADC_DATA_BUFFER_CORRUPTED))

#define LSDS_ERR_FLAG_SPI_ALL                       ((LSDS_ERR_FLAG_SPI_RX_ERROR) | \
                                                     (LSDS_ERR_FLAG_SPI_DMA_FAILURE) | \
                                                     (LSDS_ERR_FLAG_SPI_ERROR_MODE_FAULT) | \
                                                     (LSDS_ERR_FLAG_SPI_ERROR_OVERRUN) | \
                                                     (LSDS_ERR_FLAG_SPI_ERROR_FRAME_FORMAT) | \
                                                     (LSDS_ERR_FLAG_SPI_STATE_MACHINE_FAILURE) | \
                                                     (LSDS_ERR_FLAG_SPI_UNKNOWN_COMMAND_REQUESTED) | \
                                                     (LSDS_ERR_FLAG_SPI_INVALID_COMMAND_PARAMETER) | \
                                                     (LSDS_ERR_FLAG_SPI_NOT_ENOUGH_TIME_TO_PROCESS_COMMAND) | \
                                                     (LSDS_ERR_FLAG_SPI_FAILURE_WHILE_TRANSMITTING_FILLER) | \
                                                     (LSDS_ERR_FLAG_SPI_TASK_SCHEDULER_IS_BUSY))
