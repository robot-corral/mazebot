/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "stm32/stm32l4xx_ll_gpio.h"

#include "sd_impl.h"
#include "line_sensor_r1.h"

/*******************************************************************************
 * motor_controller_mc33926                                                    *
 *******************************************************************************/

extern volatile uint16_t g_leftMotorPreviousEncoderTicksA;
extern volatile uint16_t g_leftMotorPreviousEncoderTicksB;
extern volatile uint32_t g_leftMotorEncoderTicks;

extern volatile uint16_t g_rightMotorPreviousEncoderTicksA;
extern volatile uint16_t g_rightMotorPreviousEncoderTicksB;
extern volatile uint32_t g_rightMotorEncoderTicks;

/*******************************************************************************
 * imu_lsm6ds3h                                                                *
 *******************************************************************************/

typedef struct
{
    uint8_t dummy;
    uint8_t status;
    int16_t temperature;
    int16_t gyro_pitch;
    int16_t gyro_roll;
    int16_t gyro_yaw;
} __attribute__((__packed__)) ImuRawSensorData_t;

#define IMU_SPI_TX_BUFFER_LENGTH 11

extern volatile ImuRawSensorData_t g_imuRxBuffer;
extern volatile uint8_t g_imuTxBuffer[IMU_SPI_TX_BUFFER_LENGTH];

/*******************************************************************************
 * line_sensor_r1                                                              *
 *******************************************************************************/

extern volatile float g_lineDisplacementFromCenterlineInMeters;
extern volatile uint16_t g_lineSensorValues[LINE_SENSOR_COUNT];
extern volatile uint16_t g_lineSensorMinValues[LINE_SENSOR_COUNT];
extern volatile uint16_t g_lineSensorMaxValues[LINE_SENSOR_COUNT];

/*******************************************************************************
 * spi                                                                         *
 *******************************************************************************/

extern volatile uint32_t g_spiDisablePin;
extern volatile GPIO_TypeDef* g_spiDisableGpio;

/*******************************************************************************
 * exti                                                                        *
 *******************************************************************************/

extern volatile bool g_flushSdButtonPressed;
extern volatile bool g_startPauseButtonPressed;

/*******************************************************************************
 * sd                                                                          *
 *******************************************************************************/

extern volatile SdCard_t g_sdCard;
