#include "motor_control.h"

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_gpio.h>

static void initializeSlaveTimer();
static void initializeMasterTimer();

void initializeMotorControl()
{
    initializeSlaveTimer();
    initializeMasterTimer();
}

void initializeSlaveTimer()
{
    NVIC_SetPriority(TIM5_IRQn, 0);
    NVIC_EnableIRQ(TIM5_IRQn);

    LL_TIM_SetTriggerInput(TIM5, LL_TIM_TS_ITR0);
    LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_EXT_MODE1);

    LL_TIM_SetCounterMode(TIM5, LL_TIM_COUNTERMODE_DOWN);
    LL_TIM_EnableARRPreload(TIM5);

    LL_TIM_SetAutoReload(TIM5, 9);

    LL_TIM_EnableIT_UPDATE(TIM5);
}

void initializeMasterTimer()
{
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_1);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_UP);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);

    const uint32_t prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 1000000);

    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetPrescaler(TIM2, prescaler);
    LL_TIM_EnableARRPreload(TIM2);

    const uint32_t cycle = __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM2), 1000);

    LL_TIM_SetAutoReload(TIM2, cycle);
    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH | LL_TIM_OCIDLESTATE_HIGH);
    LL_TIM_OC_SetCompareCH1(TIM2, cycle / 2);
    LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);

    LL_TIM_EnableMasterSlaveMode(TIM2);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);
}

void generatePulses(diraction_t direction, uint32_t pulseCount)
{
    // TODO

    // TODO current code will generate 10 raising edges and 9 falling ones
    // TODO need to check if motor driver needs both eges to move or only falling ones
    //
    // !!! move happens on falling edge (following rising edge which is slower than 10 usec)
    //

    // direction
    // LL_GPIO_ResetOutputPin away from the motor
    // LL_GPIO_SetOutputPin towards the motor
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
    // enable
    // LL_GPIO_ResetOutputPin is enabled
    // LL_GPIO_SetOutputPin is disabled
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);

    LL_TIM_EnableCounter(TIM5);
    LL_TIM_GenerateEvent_UPDATE(TIM5);

    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
}

void TIM5_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM5) == 1)
    {
        LL_TIM_ClearFlag_UPDATE(TIM5);
        LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_6);
        LL_TIM_DisableCounter(TIM2);
    }
}
