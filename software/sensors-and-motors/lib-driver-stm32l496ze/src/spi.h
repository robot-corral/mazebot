/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "stm32/stm32l4xx_ll_gpio.h"

#define SPI_DEVICE_IMU GPIOG, LL_GPIO_PIN_12

void initializeSpi();

void flushSpi();

void setSpiBuffers(volatile void* pRxBuffer, volatile void* pTxBuffer);

void startTransmitReceiveSpi(uint32_t bufferLength, GPIO_TypeDef* pGpio, uint32_t pinMask);

bool isSpiBusy();
