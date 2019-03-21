/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "det_os.h"
#include "det_os_implementation.h"

#include <stm32/stm32l4xx_ll_bus.h>
#include <stm32/stm32l4xx_ll_pwr.h>
#include <stm32/stm32l4xx_ll_rcc.h>
#include <stm32/stm32l4xx_ll_tim.h>
#include <stm32/stm32l4xx_ll_gpio.h>
#include <stm32/stm32l4xx_ll_utils.h>
#include <stm32/stm32l4xx_ll_system.h>

static void initializeSystemClock()
{
    LL_AHB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
    LL_RCC_MSI_SetCalibTrimming(0);
    LL_RCC_MSI_Enable();

    while (LL_RCC_MSI_IsReady() != 1);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();

    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(80000000);
    LL_SetSystemCoreClock(80000000);
}

static void initializeGpio()
{
    // PG0 - pin shows duty cycle for timer task (if timer task has code to reset / set pin)

    // reset pin
    //  LDR R12, =0x48001828
    //  MOV R0, 0x1
    //  STR R0, [R12]

    // set pin
    //  LDR R12, =0x48001818
    //  MOV R0, 0x1
    //  STR R0, [R12]

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOG);

    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7 | LL_GPIO_PIN_8 | LL_GPIO_PIN_0 | LL_GPIO_PIN_1);
    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_0, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_0);
}

static void initializeOsTimer()
{
    NVIC_SetPriority(TIM2_IRQn, 15);
    NVIC_EnableIRQ(TIM2_IRQn);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    LL_TIM_SetPrescaler(TIM2, __LL_TIM_CALC_PSC(SystemCoreClock, 80000000));
    LL_TIM_SetAutoReload(TIM2, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM2), 3000));

    LL_TIM_EnableIT_UPDATE(TIM2);
}

void initializeTestTimer()
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
    LL_TIM_SetPrescaler(TIM5, __LL_TIM_CALC_PSC(SystemCoreClock, 80000000));
    LL_TIM_SetAutoReload(TIM5, 0xFFFFFFFF); // reload 32 bit value (TIM5 is 32 bit)
    LL_TIM_EnableCounter(TIM5);
    LL_TIM_GenerateEvent_UPDATE(TIM5);
}

void initializeHardware()
{
    initializeSystemClock();
    initializeGpio();
    initializeOsTimer();
    initializeTestTimer();
}

void TIM5_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM5) == 1)
    {
        LL_TIM_ClearFlag_UPDATE(TIM5);
    }
    for (;;) ;
}

void anotherTask1(void* pTaskParameter)
{
    volatile uint32_t counter = 0;
    for (;;) counter++;
}

void anotherTask2(void* pTaskParameter)
{
    volatile uint32_t counter = 0;
    for (;;) counter++;
}

void startTask(void* pTaskParameter)
{
    scheduleTask(anotherTask1, TP_NORMAL_PRIORITY, 0);
    scheduleTask(anotherTask2, TP_NORMAL_PRIORITY, 0);
    volatile uint32_t counter = 0;
    for (;;) counter++;
}

void main()
{
    initializeHardware();
    runDetOs(startTask);
}
