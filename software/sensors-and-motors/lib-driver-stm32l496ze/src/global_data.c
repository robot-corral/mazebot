/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "global_data.h"

/*******************************************************************************
 * system_clock                                                                *
 *******************************************************************************/

volatile bool g_isSystemClockInitialized = false;

/*******************************************************************************
 * motor_controller_mc33926                                                    *
 *******************************************************************************/

volatile uint16_t g_leftMotorPreviousEncoderTicksA;
volatile uint16_t g_leftMotorPreviousEncoderTicksB;
volatile uint32_t g_leftMotorEncoderTicks;

volatile uint16_t g_rightMotorPreviousEncoderTicksA;
volatile uint16_t g_rightMotorPreviousEncoderTicksB;
volatile uint32_t g_rightMotorEncoderTicks;

/*******************************************************************************
 * imu_lsm6ds3h                                                                *
 *******************************************************************************/

volatile ImuRawSensorData_t g_imuRxBuffer;
volatile uint8_t g_imuTxBuffer[IMU_SPI_TX_BUFFER_LENGTH];

/*******************************************************************************
 * line_sensor_r1                                                              *
 *******************************************************************************/

volatile float g_lineDisplacementFromCenterlineInMeters;
volatile uint16_t g_lineSensorValues[LINE_SENSOR_COUNT];
volatile uint16_t g_lineSensorMinValues[LINE_SENSOR_COUNT];
volatile uint16_t g_lineSensorMaxValues[LINE_SENSOR_COUNT];

/*******************************************************************************
 * spi                                                                         *
 *******************************************************************************/

volatile uint32_t g_spiDisablePin;
volatile GPIO_TypeDef* g_spiDisableGpio;

/*******************************************************************************
 * exti                                                                        *
 *******************************************************************************/

volatile bool g_flushSdButtonPressed;
volatile bool g_startPauseButtonPressed;

/*******************************************************************************
 * sd                                                                          *
 *******************************************************************************/

volatile uint32_t g_sdCid[4];
volatile uint32_t g_sdCsd[4];

volatile bool g_isSdHighCapacity;

volatile char* g_sdBuffer[SD_BUFFER_LENGTH];
