#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_gpio.h>
#include <stm32\stm32l4xx_ll_utils.h>

#include <mcu_arm.h>

#include "led.h"
#include "global_data.h"
#include "global_data_timer.h"
#include "position_controller.h"

/*
 * returns false if emergency stop happened true otherwise
 */
static bool positionControllerStop_Atomic()
{
    bool stopped;

    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    if (g_positionControllerXState == PCS_EMERGENCY_STOPPED)
    {
        stopped = false;
    }
    else
    {
        // disable controller first
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
        // stop counter (this will stop motor movement)
        LL_TIM_DisableCounter(TIM2);
        LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
        LL_TIM_DisableCounter(TIM5);
        // stop acceleration dma
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        setPositionControllerMovingLedEnabled(false);
        stopped = true;
    }

    setInterruptMask(oldInterruptMask);

    return stopped;
}

static positionControllerState_t positionControllerMoveAtConstantSpeed_Atomic(positionControllerState_t desiredState)
{
    positionControllerState_t result;

    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    if (g_positionControllerXState == PCS_EMERGENCY_STOPPED)
    {
        result = PCS_EMERGENCY_STOPPED;
    }
    else
    {
        g_positionControllerXState = result = desiredState;

        // enable motor controller
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
        // make sure to wait until motor is actually enabled before starting signal generation
        // (enable pin has capacitor so it takes time to charge)
        LL_mDelay(1);

        // need to load initial timer register values as DMA is only triggered on 2nd step
        LL_TIM_SetAutoReload(TIM2, g_dmaTimerDataIncreasing[0]);
        LL_TIM_SetRepetitionCounter(TIM2, g_dmaTimerDataIncreasing[1]);
        LL_TIM_OC_SetCompareCH1(TIM2, g_dmaTimerDataIncreasing[2]);

        // reset counter to 0 so we can count number of steps motor moves
        LL_TIM_SetCounter(TIM5, 0);
        // get ready to count TIM2 pulses
        LL_TIM_EnableCounter(TIM5);
        // get ready to generate motor control signal
        LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
        LL_TIM_EnableCounter(TIM2);
        // force start signal generation
        // 1st step uses data which was loaded into ARR/CCR1 registers
        // 2nd step and on will be updated from DMA
        LL_TIM_GenerateEvent_UPDATE(TIM2);
    }

    setInterruptMask(oldInterruptMask);

    return result;
}

static positionControllerState_t positionControllerMoveWithAcceleration_Atomic(positionControllerState_t desiredState, uint32_t stepsCount)
{
    positionControllerState_t result;

    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    if (g_positionControllerXState == PCS_EMERGENCY_STOPPED)
    {
        result = PCS_EMERGENCY_STOPPED;
    }
    else
    {
        g_positionControllerXState = result = desiredState;

        // enable motor controller
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
        // make sure to wait until motor is actually enabled before starting signal generation
        // (enable pin has capacitor so it takes time to charge)
        LL_mDelay(1);

        // need to load initial timer register values as DMA is only triggered on 2nd step
        LL_TIM_SetAutoReload(TIM2, g_dmaTimerDataIncreasing[0]);
        LL_TIM_SetRepetitionCounter(TIM2, g_dmaTimerDataIncreasing[1]);
        LL_TIM_OC_SetCompareCH1(TIM2, g_dmaTimerDataIncreasing[2]);

        LL_DMA_ConfigAddresses(DMA1,
                               LL_DMA_CHANNEL_2,
                               (uint32_t) &g_dmaTimerDataIncreasing[3], // skip 1st step as it will be generated based on data loaded into ARR/CCR1 registers
                               (uint32_t) &TIM2->DMAR, // we are using DMA burst to update several TIM2 register at once (ARR, RCR, CCR1)
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        // even though we skipped 1 step in DMA (starting from 3) we need to add one more step at the end
        // as DMA generates Transfer Complete interrupt after data is loaded into timer registers but before this data is used by the timer
        // it is safe to do so as g_dmaTimerDataIncreasing has extra data at the end to account for this
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, stepsCount * 3);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

        // reset counter to 0 so we can count number of steps motor moves
        LL_TIM_SetCounter(TIM5, 0);
        // get ready to count TIM2 pulses
        LL_TIM_EnableCounter(TIM5);
        // get ready to generate motor control signal
        LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
        LL_TIM_EnableCounter(TIM2);
        // force start signal generation
        // 1st step uses data which was loaded into ARR/CCR1 registers
        // 2nd step and on will be updated from DMA
        LL_TIM_GenerateEvent_UPDATE(TIM2);
    }

    setInterruptMask(oldInterruptMask);

    return result;
}

void startSlowingDown_Atomic(uint32_t slowDownPulseCount)
{
    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    if (g_positionControllerXState != PCS_EMERGENCY_STOPPED)
    {
        g_positionControllerXState = PCS_BUSY_SLOWING_DOWN;

        const uint32_t startIndex = (DMA_TIMER_STEPS_COUNT - g_slowDownPulseCount) * 3;

        LL_DMA_ConfigAddresses(DMA1,
                               LL_DMA_CHANNEL_2,
                               (uint32_t) &g_dmaTimerDataDecreasing[startIndex],
                               (uint32_t) &TIM2->DMAR, // we are using DMA burst to update several TIM2 register at once (ARR, RCR, CCR1)
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        // we need to add TWO extra DMA data items
        //
        // TODO
        // TODO need to figure out is it 2 and not 1
        // TODO
        //
        // as DMA generates Transfer Complete interrupt after data is loaded into timer registers but before this data is used by the timer
        // it is safe to do so as g_dmaTimerDataIncreasing has extra data at the end to account for this
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, (g_slowDownPulseCount + 2) * 3);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    }

    setInterruptMask(oldInterruptMask);
}

/*
 * returns false if emergency stop happened and true if desired state was set
 */
bool setPositionControllerXState_Atomic(positionControllerState_t desiredState)
{
    bool result;

    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    if (g_positionControllerXState == PCS_EMERGENCY_STOPPED)
    {
        result = false;
    }
    else
    {
        result = true;
        g_positionControllerXState = desiredState;
    }

    setInterruptMask(oldInterruptMask);

    return result;
}
