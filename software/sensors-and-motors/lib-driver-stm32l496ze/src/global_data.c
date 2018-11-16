/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

/*******************************************************************************
 * system_clock                                                                *
 *******************************************************************************/

bool g_isSystemClockInitialized = false;

/*******************************************************************************
 * motor_controller_mc33926                                                    *
 *******************************************************************************/

uint16_t g_leftMotorPreviousEncoderTicksA;
uint16_t g_leftMotorPreviousEncoderTicksB;
uint32_t g_leftMotorEncoderTicks;

uint16_t g_rightMotorPreviousEncoderTicksA;
uint16_t g_rightMotorPreviousEncoderTicksB;
uint32_t g_rightMotorEncoderTicks;

/*******************************************************************************
 * imu_lsm6ds3h                                                                *
 *******************************************************************************/

volatile ImuRawSensorData_t g_imuRxBuffer;
volatile uint8_t g_imuTxBuffer[IMU_SPI_TX_BUFFER_LENGTH];

/*******************************************************************************
 * line_sensor_r1                                                              *
 *******************************************************************************/

volatile uint16_t g_lineSensorValues[LINE_SENSOR_COUNT];

/*******************************************************************************
 * spi                                                                         *
 *******************************************************************************/

uint32_t g_spiDisablePin;
GPIO_TypeDef* g_spiDisableGpio;
