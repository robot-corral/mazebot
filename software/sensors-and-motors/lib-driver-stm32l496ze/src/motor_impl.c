/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_tim.h"
#include "stm32/stm32l4xx_ll_gpio.h"
#include "stm32/stm32l4xx_ll_lptim.h"

#include "gpio.h"
#include "timers.h"
#include "parameters.h"
#include "global_data.h"
#include "rolling_values.h"

#include "motor.h"

void initializeMotor()
{
    g_leftMotorPreviousEncoderTicksA = 0;
    g_leftMotorPreviousEncoderTicksB = 0;
    g_leftMotorEncoderTicks = 0;

    g_rightMotorPreviousEncoderTicksA = 0;
    g_rightMotorPreviousEncoderTicksB = 0;
    g_rightMotorEncoderTicks = 0;
}

void enableLeftMotor()
{
    LL_GPIO_ResetOutputPin(LEFT_MOTOR_D1_GPIO, LEFT_MOTOR_D1_PIN);
    LL_GPIO_SetOutputPin(LEFT_MOTOR_D2_GPIO, LEFT_MOTOR_D2_PIN);
    LL_GPIO_SetOutputPin(LEFT_MOTOR_EN_GPIO, LEFT_MOTOR_EN_PIN);
}

void setLeftMotorPower(int16_t power)
{
    if (power > 0)
    {
        LL_TIM_OC_SetCompareCH1(TIM16, power);
        LL_TIM_OC_SetCompareCH1(TIM17, 0);
    }
    else if (power < 0)
    {
        LL_TIM_OC_SetCompareCH1(TIM16, 0);
        LL_TIM_OC_SetCompareCH1(TIM17, -power);
    }
    else
    {
        LL_TIM_OC_SetCompareCH1(TIM16, 0);
        LL_TIM_OC_SetCompareCH1(TIM17, 0);
    }
}

uint32_t getLeftMotorEncoderTicks()
{
    const uint16_t newValueA = LL_TIM_GetCounter(TIM1);
    const uint16_t newValueB = LL_TIM_GetCounter(TIM3);
    const uint16_t delta1 = calculateRolling16BitValueDifference(g_leftMotorPreviousEncoderTicksA, newValueA);
    const uint16_t delta2 = calculateRolling16BitValueDifference(g_leftMotorPreviousEncoderTicksB, newValueB);
    g_leftMotorPreviousEncoderTicksA = newValueA;
    g_leftMotorPreviousEncoderTicksB = newValueB;
    g_leftMotorEncoderTicks += delta1 + delta2;
    return g_leftMotorEncoderTicks;
}

void disableLeftMotor()
{
    LL_GPIO_ResetOutputPin(LEFT_MOTOR_EN_GPIO, LEFT_MOTOR_EN_PIN);
}

void enableRightMotor()
{
    LL_GPIO_ResetOutputPin(RIGHT_MOTOR_D1_GPIO, RIGHT_MOTOR_D1_PIN);
    LL_GPIO_SetOutputPin(RIGHT_MOTOR_D2_GPIO, RIGHT_MOTOR_D2_PIN);
    LL_GPIO_SetOutputPin(RIGHT_MOTOR_EN_GPIO, RIGHT_MOTOR_EN_PIN);
}

void setRightMotorPower(int16_t power)
{
    if (power > 0)
    {
        while (!LL_LPTIM_IsActiveFlag_CMPOK(LPTIM1)) ;
        LL_LPTIM_SetCompare(LPTIM1, MAX_MOTOR_POWER);
        while (!LL_LPTIM_IsActiveFlag_CMPOK(LPTIM2)) ;
        LL_LPTIM_SetCompare(LPTIM2, MAX_MOTOR_POWER - power);
    }
    else if (power < 0)
    {
        while (!LL_LPTIM_IsActiveFlag_CMPOK(LPTIM1)) ;
        LL_LPTIM_SetCompare(LPTIM1, MAX_MOTOR_POWER + power);
        while (!LL_LPTIM_IsActiveFlag_CMPOK(LPTIM2)) ;
        LL_LPTIM_SetCompare(LPTIM2, MAX_MOTOR_POWER);
    }
    else
    {
        while (!LL_LPTIM_IsActiveFlag_CMPOK(LPTIM1)) ;
        LL_LPTIM_SetCompare(LPTIM1, MAX_MOTOR_POWER);
        while (!LL_LPTIM_IsActiveFlag_CMPOK(LPTIM2)) ;
        LL_LPTIM_SetCompare(LPTIM2, MAX_MOTOR_POWER);
    }
}

uint32_t getRightMotorEncoderTicks()
{
    const uint16_t newValueA = LL_TIM_GetCounter(TIM4);
    const uint16_t newValueB = LL_TIM_GetCounter(TIM15);
    const uint16_t delta1 = calculateRolling16BitValueDifference(g_rightMotorPreviousEncoderTicksA, newValueA);
    const uint16_t delta2 = calculateRolling16BitValueDifference(g_rightMotorPreviousEncoderTicksB, newValueB);
    g_rightMotorPreviousEncoderTicksA = newValueA;
    g_rightMotorPreviousEncoderTicksB = newValueB;
    g_rightMotorEncoderTicks += delta1 + delta2;
    return g_rightMotorEncoderTicks;
}

void disableRightMotor()
{
    LL_GPIO_ResetOutputPin(RIGHT_MOTOR_EN_GPIO, RIGHT_MOTOR_EN_PIN);
}
