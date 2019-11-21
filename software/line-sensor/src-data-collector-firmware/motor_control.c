#include "motor_control.h"

#include "global_data.h"
#include "interrupt_priorities.h"

#include <stdatomic.h>

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_gpio.h>
#include <stm32\stm32l4xx_ll_utils.h>

static void initializeSlaveTimer();
static void initializeMasterTimer();

void initializeMotorControl()
{
    atomic_store(&g_isMotorControlBusy, false);
    initializeSlaveTimer();
    initializeMasterTimer();
}

void initializeSlaveTimer()
{
    NVIC_SetPriority(TIM5_IRQn, IRQ_PRIORITY_MOTOR_PWM_STOP);
    NVIC_EnableIRQ(TIM5_IRQn);

    LL_TIM_SetTriggerInput(TIM5, LL_TIM_TS_ITR0);
    LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_EXT_MODE1);

    LL_TIM_SetCounterMode(TIM5, LL_TIM_COUNTERMODE_DOWN);
    LL_TIM_EnableARRPreload(TIM5);

    LL_TIM_EnableIT_UPDATE(TIM5);
}

void initializeMasterTimer()
{
    const uint32_t prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 1000000);

    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetPrescaler(TIM2, prescaler);
    LL_TIM_EnableARRPreload(TIM2);

    const uint32_t cycle = __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM2), 1000);

    LL_TIM_SetAutoReload(TIM2, cycle);
    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    // timer update is on falling edge
    // idle timer state is low
    // (as motor controller expects rising edge, then falling edge on which it moves the motor)
    LL_TIM_OC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_LOW | LL_TIM_OCIDLESTATE_LOW);
    LL_TIM_OC_SetCompareCH1(TIM2, cycle / 2);
    LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);

    LL_TIM_EnableMasterSlaveMode(TIM2);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);
}

bool isBusy()
{
    return atomic_load(&g_isMotorControlBusy);
}

bool generatePulses(diraction_t direction, uint32_t pulseCount)
{
    bool expected = false;

    if (!atomic_compare_exchange_strong(&g_isMotorControlBusy, &expected, true))
    {
        return false;
    }

    switch (direction)
    {
        case D_FORWARD:
        {
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
            break;
        }
        case D_BACKWARD:
        {
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
            break;
        }
        default:
        {
            return false;
        }
    }

    LL_TIM_SetAutoReload(TIM5, pulseCount);
    LL_TIM_SetCounter(TIM5, pulseCount);
    LL_TIM_EnableCounter(TIM5);

    // enable motor controller
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);

    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);

    return true;
}

void TIM5_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM5) == 1)
    {
        LL_TIM_ClearFlag_UPDATE(TIM5);
        LL_TIM_DisableCounter(TIM2);
        LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
        // wait 1 millisecond for falling edge to move the motor
        // (might need to reduce this wait time in the future)
        LL_mDelay(1);
        // disable motor controller
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
        atomic_store(&g_isMotorControlBusy, false);
    }
}

void emergencyStop()
{
    // disable controller first
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
    // disable timer after that
    LL_TIM_ClearFlag_UPDATE(TIM5);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    // no longer busy!
    atomic_store(&g_isMotorControlBusy, false);
}
