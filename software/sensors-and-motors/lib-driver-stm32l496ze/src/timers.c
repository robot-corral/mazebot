/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_bus.h"
#include "stm32/stm32l4xx_ll_rcc.h"
#include "stm32/stm32l4xx_ll_tim.h"
#include "stm32/stm32l4xx_ll_lptim.h"

#include "timers.h"

static void initializeLeftMotorEncoderA();
static void initializeLeftMotorEncoderB();
static void initializeRightMotorEncoderA();
static void initializeRightMotorEncoderB();
static void initializeLeftMotorPwm1();
static void initializeLeftMotorPwm2();
static void initializeRightMotorPwm1();
static void initializeRightMotorPwm2();

void initializeTimers()
{
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM1_CLKSOURCE_LSI);
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM2_CLKSOURCE_LSI);

    initializeLeftMotorEncoderA();
    initializeLeftMotorEncoderB();

    initializeRightMotorEncoderA();
    initializeRightMotorEncoderB();

    initializeLeftMotorPwm1();
    initializeLeftMotorPwm2();

    initializeRightMotorPwm1();
    initializeRightMotorPwm2();
}

void initializeLeftMotorEncoderA()
{
    // Timer 1, Channel 1, Pin PE9

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM1, LL_TIM_CLOCKDIVISION_DIV1);

    LL_TIM_DisableARRPreload(TIM1);
    LL_TIM_SetTriggerInput(TIM1, LL_TIM_TS_TI1FP1);
    LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_EXT_MODE1);
    LL_TIM_IC_SetFilter(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_BOTHEDGE);
    LL_TIM_DisableIT_TRIG(TIM1);
    LL_TIM_DisableDMAReq_TRIG(TIM1);
    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
    LL_TIM_SetTriggerOutput2(TIM1, LL_TIM_TRGO2_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM1);

    LL_TIM_SetPrescaler(TIM1, 0);
    LL_TIM_SetRepetitionCounter(TIM1, 0);
    LL_TIM_SetAutoReload(TIM1, 0xFFFF);
    LL_TIM_EnableCounter(TIM1);
}

void initializeLeftMotorEncoderB()
{
    // Timer 3, Channel 1, Pin PE3

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

    LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM3, LL_TIM_CLOCKDIVISION_DIV1);

    LL_TIM_DisableARRPreload(TIM3);
    LL_TIM_SetTriggerInput(TIM3, LL_TIM_TS_TI1FP1);
    LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_EXT_MODE1);
    LL_TIM_IC_SetFilter(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_BOTHEDGE);
    LL_TIM_DisableIT_TRIG(TIM3);
    LL_TIM_DisableDMAReq_TRIG(TIM3);
    LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
    LL_TIM_SetTriggerOutput2(TIM3, LL_TIM_TRGO2_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM3);

    LL_TIM_SetPrescaler(TIM3, 0);
    LL_TIM_SetRepetitionCounter(TIM3, 0);
    LL_TIM_SetAutoReload(TIM3, 0xFFFF);
    LL_TIM_EnableCounter(TIM3);
}

void initializeRightMotorEncoderA()
{
    // Timer 4, Channel 1, Pin PD12

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

    LL_TIM_SetCounterMode(TIM4, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM4, LL_TIM_CLOCKDIVISION_DIV1);

    LL_TIM_DisableARRPreload(TIM4);
    LL_TIM_SetTriggerInput(TIM4, LL_TIM_TS_TI1FP1);
    LL_TIM_SetClockSource(TIM4, LL_TIM_CLOCKSOURCE_EXT_MODE1);
    LL_TIM_IC_SetFilter(TIM4, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM4, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_BOTHEDGE);
    LL_TIM_DisableIT_TRIG(TIM4);
    LL_TIM_DisableDMAReq_TRIG(TIM4);
    LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
    LL_TIM_SetTriggerOutput2(TIM4, LL_TIM_TRGO2_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM4);

    LL_TIM_SetPrescaler(TIM4, 0);
    LL_TIM_SetRepetitionCounter(TIM4, 0);
    LL_TIM_SetAutoReload(TIM4, 0xFFFF);
    LL_TIM_EnableCounter(TIM4);
}

void initializeRightMotorEncoderB()
{
    // Timer 15, Channel 1, Pin PB14

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM15);

    LL_TIM_SetCounterMode(TIM15, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM15, LL_TIM_CLOCKDIVISION_DIV1);

    LL_TIM_DisableARRPreload(TIM15);
    LL_TIM_SetTriggerInput(TIM15, LL_TIM_TS_TI1FP1);
    LL_TIM_SetClockSource(TIM15, LL_TIM_CLOCKSOURCE_EXT_MODE1);
    LL_TIM_IC_SetFilter(TIM15, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM15, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_BOTHEDGE);
    LL_TIM_DisableIT_TRIG(TIM15);
    LL_TIM_DisableDMAReq_TRIG(TIM15);
    LL_TIM_SetTriggerOutput(TIM15, LL_TIM_TRGO_RESET);
    LL_TIM_SetTriggerOutput2(TIM15, LL_TIM_TRGO2_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM15);

    LL_TIM_SetPrescaler(TIM15, 0);
    LL_TIM_SetRepetitionCounter(TIM15, 0);
    LL_TIM_SetAutoReload(TIM15, 0xFFFF);
    LL_TIM_EnableCounter(TIM15);
}

void initializeLeftMotorPwm1()
{
    // Timer 16, Channel 1, Pin PE0

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM16);

    LL_TIM_SetPrescaler(TIM16, __LL_TIM_CALC_PSC(SystemCoreClock, 32050));
    LL_TIM_SetAutoReload(TIM16, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM16), 32));

    LL_TIM_OC_SetMode(TIM16, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM16, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_SetCompareCH1(TIM16, 500);

    LL_TIM_OC_EnablePreload(TIM16, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableAutomaticOutput(TIM16);
    LL_TIM_CC_EnableChannel(TIM16, LL_TIM_CHANNEL_CH1);

    LL_TIM_EnableCounter(TIM16);
    LL_TIM_GenerateEvent_UPDATE(TIM16);
}

void initializeLeftMotorPwm2()
{
    // Timer 17, Channel 1, Pin PE1

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM17); 

    LL_TIM_SetPrescaler(TIM17, __LL_TIM_CALC_PSC(SystemCoreClock, 32050));
    LL_TIM_SetAutoReload(TIM17, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM17), 32));

    LL_TIM_OC_SetMode(TIM17, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM17, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_SetCompareCH1(TIM17, 500);

    LL_TIM_OC_EnablePreload(TIM17, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableAutomaticOutput(TIM17);
    LL_TIM_CC_EnableChannel(TIM17, LL_TIM_CHANNEL_CH1);

    LL_TIM_EnableCounter(TIM17);
    LL_TIM_GenerateEvent_UPDATE(TIM17);
}

void initializeRightMotorPwm1()
{
    // LPTIM1, Pin PB2

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);

    LL_LPTIM_SetClockSource(LPTIM1, LL_LPTIM_CLK_SOURCE_INTERNAL);
    LL_LPTIM_SetPrescaler(LPTIM1, LL_LPTIM_PRESCALER_DIV1);
    LL_LPTIM_SetPolarity(LPTIM1, LL_LPTIM_OUTPUT_POLARITY_REGULAR);
    LL_LPTIM_SetUpdateMode(LPTIM1, LL_LPTIM_UPDATE_MODE_IMMEDIATE);
    LL_LPTIM_SetCounterMode(LPTIM1, LL_LPTIM_COUNTER_MODE_INTERNAL);
    LL_LPTIM_TrigSw(LPTIM1);

    LL_LPTIM_ConfigOutput(LPTIM1, LL_LPTIM_OUTPUT_WAVEFORM_PWM, LL_LPTIM_OUTPUT_POLARITY_REGULAR);
    LL_LPTIM_Enable(LPTIM1);

    LL_LPTIM_SetAutoReload(LPTIM1, 1000);
    LL_LPTIM_SetCompare(LPTIM1, 1000);

    LL_LPTIM_StartCounter(LPTIM1, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}

void initializeRightMotorPwm2()
{
    // LPTIM2, Pin PD13

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPTIM2);

    LL_LPTIM_SetClockSource(LPTIM2, LL_LPTIM_CLK_SOURCE_INTERNAL);
    LL_LPTIM_SetPrescaler(LPTIM2, LL_LPTIM_PRESCALER_DIV1);
    LL_LPTIM_SetPolarity(LPTIM2, LL_LPTIM_OUTPUT_POLARITY_REGULAR);
    LL_LPTIM_SetUpdateMode(LPTIM2, LL_LPTIM_UPDATE_MODE_IMMEDIATE);
    LL_LPTIM_SetCounterMode(LPTIM2, LL_LPTIM_COUNTER_MODE_INTERNAL);
    LL_LPTIM_TrigSw(LPTIM2);

    LL_LPTIM_ConfigOutput(LPTIM2, LL_LPTIM_OUTPUT_WAVEFORM_PWM, LL_LPTIM_OUTPUT_POLARITY_REGULAR);
    LL_LPTIM_Enable(LPTIM2);

    LL_LPTIM_SetAutoReload(LPTIM2, 1000);
    LL_LPTIM_SetCompare(LPTIM2, 1000);

    LL_LPTIM_StartCounter(LPTIM2, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}
