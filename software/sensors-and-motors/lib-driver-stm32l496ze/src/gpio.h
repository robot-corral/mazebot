/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdbool.h>

#define LED_ERROR_0_GPIO GPIOB
#define LED_ERROR_0_PIN  LL_GPIO_PIN_15

#define LED_ERROR_1_GPIO GPIOD
#define LED_ERROR_1_PIN  LL_GPIO_PIN_8

#define LED_POWER_ON_GPIO GPIOE
#define LED_POWER_ON_PIN  LL_GPIO_PIN_5

#define LEFT_MOTOR_D1_GPIO GPIOF
#define LEFT_MOTOR_D1_PIN  LL_GPIO_PIN_1

#define LEFT_MOTOR_D2_GPIO GPIOF
#define LEFT_MOTOR_D2_PIN  LL_GPIO_PIN_2

#define LEFT_MOTOR_EN_GPIO GPIOC
#define LEFT_MOTOR_EN_PIN  LL_GPIO_PIN_13

#define RIGHT_MOTOR_D1_GPIO GPIOB
#define RIGHT_MOTOR_D1_PIN  LL_GPIO_PIN_12

#define RIGHT_MOTOR_D2_GPIO GPIOB
#define RIGHT_MOTOR_D2_PIN  LL_GPIO_PIN_13

#define RIGHT_MOTOR_EN_GPIO GPIOB
#define RIGHT_MOTOR_EN_PIN  LL_GPIO_PIN_10

void initializeGpio();
void initializeDebugGpio(bool enable);
