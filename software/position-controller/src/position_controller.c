#include "position_controller.h"

#include "led.h"
#include "global_data.h"
#include "global_data_timer.h"
#include "interrupt_priorities.h"

#include <mcu_arm.h>

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_gpio.h>
#include <stm32\stm32l4xx_ll_utils.h>

static void initializeSlaveTimer();
static void initializeMasterTimer();
static void initializeMasterTimerDma();

static void positionControllerStop_Unsafe();
static void positionControllerEmergencyStop_Unsafe();
static void positionControllerMoveUntilLimitSwitchTriggers_Unsafe(positionControllerDirection_t direction);
static void positionControllerMove_Unsafe(positionControllerDirection_t direction, uint32_t pulseCount);

void initializePositionController()
{
    g_positionControllerX = 0;
    g_positionControllerXMaxValue = 0;
    g_positionControllerXPulseErrorCount = 0;
    g_positionControllerXPlannedPulseCount = 0;

    g_positionControllerXState = PCS_RESET;
    g_positionControllerXDirection = PCD_NONE;

    initializeMasterTimerDma();
    initializeSlaveTimer();
    initializeMasterTimer();
}

void initializeSlaveTimer()
{
    NVIC_SetPriority(TIM5_IRQn, IRQ_PRIORITY_MOTOR_PWM_STOP);
    NVIC_EnableIRQ(TIM5_IRQn);

    // trigger input is from:
    // RM0351 Reference manual - STM32L4x5 and STM32L4x6 advanced ARM®-based 32-bit MCUs
    // 31.4.3 TIMx slave mode control register (TIMx_SMCR)
    // Table 192. TIMx internal trigger connection
    LL_TIM_SetTriggerInput(TIM5, LL_TIM_TS_ITR0);
    // see:
    // 31.3.3 Clock selection
    // External clock source mode 1
    // Figure 293. TI2 external clock connection example
    LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_EXT_MODE1);

    LL_TIM_SetCounterMode(TIM5, LL_TIM_COUNTERMODE_UP);
    LL_TIM_EnableARRPreload(TIM5);
    LL_TIM_SetAutoReload(TIM5, 0);
}

void initializeMasterTimer()
{
    const uint32_t prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 40000000);

    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetPrescaler(TIM2, prescaler);
    LL_TIM_EnableARRPreload(TIM2);

    LL_TIM_SetAutoReload(TIM2, DMA_TIMER_MIN_FREQUENCY_ARR);
    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    // timer update is on falling edge
    // idle timer state is low
    // (as motor controller expects rising edge, then falling edge on which it moves the motor)
    LL_TIM_OC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_LOW | LL_TIM_OCIDLESTATE_LOW);
    LL_TIM_OC_SetCompareCH1(TIM2, DMA_TIMER_MIN_FREQUENCY_CC);
    LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);

    // TIM2 is going to drive TIM5
    // TIM2 generates PWMs (1/2 cycle it is 0 and 1/2 cycle is 1)
    // TIM5 counts number of PWM pulses
    LL_TIM_EnableMasterSlaveMode(TIM2);
    // this is from:
    // 31.4.2 TIMx control register 2 (TIMx_CR2)
    // 010: Update - The update event is selected as trigger output (TRGO). For instance a master 
    // timer can then be used as a prescaler for a slave timer.
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);

    LL_TIM_ConfigDMABurst(TIM2, LL_TIM_DMABURST_BASEADDR_ARR, LL_TIM_DMABURST_LENGTH_3TRANSFERS);
    LL_TIM_EnableDMAReq_UPDATE(TIM2);
}

void initializeMasterTimerDma()
{
    NVIC_SetPriority(DMA1_Channel2_IRQn, IRQ_PRIORITY_MOTOR_PWM_DMA);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    LL_DMA_ConfigTransfer(DMA1,
                          LL_DMA_CHANNEL_2,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_MODE_NORMAL                |
                          LL_DMA_PERIPH_NOINCREMENT         |
                          LL_DMA_MEMORY_INCREMENT           |
                          LL_DMA_PDATAALIGN_WORD            |
                          LL_DMA_MDATAALIGN_WORD            |
                          LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_4);

    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_2,
                           (uint32_t) &g_dmaTimerDataIncreasing[3],
                           (uint32_t) &TIM2->DMAR,
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, (DMA_TIMER_STEPS_COUNT - 1) * 3);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

uint32_t getPosition()
{
    return g_positionControllerX;
}

positionControllerState_t getState()
{
    return g_positionControllerXState;
}

uint32_t getAbsolutePositionError()
{
    return g_positionControllerXPulseErrorCount;
}

moveRequestResult_t calibratePositionController(positionControllerState_t* pState)
{
    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    moveRequestResult_t result;
    positionControllerState_t positionControllerXState = g_positionControllerXState;

    if (positionControllerXState == PCS_RESET || positionControllerXState == PCS_IDLE)
    {
        // move to min position 1st
        g_positionControllerXDirection = PCD_BACKWARD;
        g_positionControllerXState = PCS_BUSY_CALIBRATING_MIN;
        positionControllerXState = PCS_BUSY_CALIBRATING_MIN;
        positionControllerMoveUntilLimitSwitchTriggers_Unsafe(PCD_BACKWARD);
        result = MRR_OK;
    }
    else if ((positionControllerXState & PCS_BUSY) == PCS_BUSY)
    {
        result = MRR_BUSY;
    }
    else
    {
        result = MRR_INVALID_STATE;
    }

    setInterruptMask(oldInterruptMask);

    if (pState)
    {
        *pState = positionControllerXState;
    }

    return result;
}

moveRequestResult_t setPosition(positionControllerDirection_t direction, uint32_t pulseCount, positionControllerState_t* pState)
{
    if (pulseCount == 0)
    {
        return MRR_OK; // no-op command
    }

    positionControllerState_t desired;

    switch (direction)
    {
        case PCD_FORWARD  : desired = PCS_BUSY_MOVING_FORWARD; break;
        case PCD_BACKWARD : desired = PCS_BUSY_MOVING_BACKWARD; break;
        default           :
        {
            if (pState)
            {
                *pState = g_positionControllerXState;
            }
            return MRR_INVALID_PARAMETER;
        }
    }

    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    moveRequestResult_t result;
    positionControllerState_t positionControllerXState = g_positionControllerXState;

    if (positionControllerXState == PCS_IDLE)
    {
        switch (direction)
        {
            case PCD_FORWARD:
            {
                // make sure we don't trigger max limit-switch
                if (pulseCount > g_positionControllerXMaxValue || g_positionControllerX > g_positionControllerXMaxValue - pulseCount)
                {
                    result = MRR_INVALID_PARAMETER;
                }
                else
                {
                    g_positionControllerXState = PCS_BUSY_MOVING_FORWARD;
                    positionControllerMove_Unsafe(PCD_FORWARD, pulseCount);
                }
                break;
            }
            case PCD_BACKWARD:
            {
                // make sure we don't trigger min limit-switch
                if (pulseCount > g_positionControllerX)
                {
                    result = MRR_INVALID_PARAMETER;
                }
                else
                {
                    g_positionControllerXState = PCS_BUSY_MOVING_BACKWARD;
                    positionControllerMove_Unsafe(PCD_BACKWARD, pulseCount);
                }
                break;
            }
            default:
            {
                // something went horribly wrong if we ended up here
                positionControllerEmergencyStop_Unsafe();
                result = MRR_UNEXPECTED_ERROR;
                positionControllerXState = PCS_EMERGENCY_STOPPED;
                break;
            }
        }
    }
    else if ((positionControllerXState & PCS_BUSY) == PCS_BUSY)
    {
        result = MRR_BUSY;
    }
    else
    {
        result = MRR_INVALID_STATE;
    }

    setInterruptMask(oldInterruptMask);

    if (pState)
    {
        *pState = positionControllerXState;
    }

    return result;
}

void positionControllerEmergencyStop()
{
    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    positionControllerEmergencyStop_Unsafe();

    setInterruptMask(oldInterruptMask);
}

void positionControllerLimitStop(positionControllerLimitStopType_t limitStopType)
{
    const uint32_t oldInterruptMask = getInterruptMask();
    disableInterrupts();

    bool performEmergencyStop = true;
    const positionControllerState_t positionControllerXState = g_positionControllerXState;

    if (positionControllerXState == PCS_BUSY_CALIBRATING_MIN)
    {
        if (limitStopType == PCLST_MIN)
        {
            positionControllerStop_Unsafe();
            // restart counting from 0
            LL_TIM_SetCounter(TIM5, 0);
            // move to max position
            g_positionControllerXState = PCS_BUSY_CALIBRATING_MAX;
            positionControllerMoveUntilLimitSwitchTriggers_Unsafe(PCD_FORWARD);
            performEmergencyStop = false;
        }
        else
        {
            // ignore max limit switch as we are moving away from it
            performEmergencyStop = false;
        }
    }
    else if (positionControllerXState == PCS_BUSY_CALIBRATING_MAX)
    {
        if (limitStopType == PCLST_MIN)
        {
            // make sure to reset 0 position if limit switch got triggered again
            LL_TIM_SetCounter(TIM5, 0);
            // ignore min limit switch as we are moving away from it
            performEmergencyStop = false;
        }
        else if (limitStopType == PCLST_MAX)
        {
            positionControllerStop_Unsafe();
            // remember max position
            g_positionControllerXMaxValue = LL_TIM_GetCounter(TIM5);
            g_positionControllerX = g_positionControllerXMaxValue;
            g_positionControllerXState = PCS_BUSY_CALIBRATING_CORRECT_MAX;
            // move to the middle (this gives us opportunity to get away from
            // max limit switch and update max position if it triggers again
            positionControllerMove_Unsafe(PCD_BACKWARD, g_positionControllerXMaxValue / 2);
            performEmergencyStop = false;
        }
    }
    else if (positionControllerXState == PCS_BUSY_CALIBRATING_CORRECT_MAX)
    {
        if (limitStopType == PCLST_MAX)
        {
            // remember how much did we move before max limit switch triggered again
            const uint32_t delta = LL_TIM_GetCounter(TIM5);
            // restart counting
            LL_TIM_SetCounter(TIM5, 0);
            if (delta <= g_positionControllerXMaxValue)
            {
                // update max position
                g_positionControllerXMaxValue -= delta;
                g_positionControllerX -= delta;
                // ignore max limit switch as we are moving away from it
                performEmergencyStop = false;
            }
        }
    }

    if (performEmergencyStop)
    {
        // otherwise something went wrong and some limit switch got triggered
        // so perform emergency stop
        positionControllerEmergencyStop_Unsafe();
    }

    setInterruptMask(oldInterruptMask);
}

void NMI_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void HardFault_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void MemManage_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void BusFault_Handler()
{
    positionControllerEmergencyStop();
    setFatalErrorLedEnabled(true);
    for (;;) ;
}

void TIM5_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_CC1(TIM5) == 1)
    {
        const uint32_t oldInterruptMask = getInterruptMask();
        disableInterrupts();

        positionControllerStop_Unsafe();

        const uint32_t actualPulsesProduced = LL_TIM_GetCounter(TIM5);
        const positionControllerState_t positionControllerXState = g_positionControllerXState;

        if (positionControllerXState == PCS_BUSY_MOVING_FORWARD ||
            positionControllerXState == PCS_BUSY_MOVING_BACKWARD ||
            positionControllerXState == PCS_BUSY_CALIBRATING_CORRECT_MAX)
        {
            g_positionControllerXState = PCS_IDLE;

            switch (g_positionControllerXDirection)
            {
                case PCD_FORWARD:
                {
                    if (actualPulsesProduced > g_positionControllerXPlannedPulseCount)
                    {
                        g_positionControllerXPulseErrorCount += actualPulsesProduced - g_positionControllerXPlannedPulseCount;
                    }
                    else if (actualPulsesProduced < g_positionControllerXPlannedPulseCount)
                    {
                        g_positionControllerXPulseErrorCount += g_positionControllerXPlannedPulseCount - actualPulsesProduced;
                    }
                    if (actualPulsesProduced <= g_positionControllerXMaxValue &&
                        g_positionControllerX <= g_positionControllerXMaxValue - actualPulsesProduced)
                    {
                        g_positionControllerX += actualPulsesProduced;
                    }
                    else
                    {
                        positionControllerEmergencyStop_Unsafe();
                    }
                    break;
                }
                case PCD_BACKWARD:
                {
                    if (actualPulsesProduced > g_positionControllerXPlannedPulseCount)
                    {
                        g_positionControllerXPulseErrorCount += actualPulsesProduced - g_positionControllerXPlannedPulseCount;
                    }
                    else if (actualPulsesProduced < g_positionControllerXPlannedPulseCount)
                    {
                        g_positionControllerXPulseErrorCount += g_positionControllerXPlannedPulseCount - actualPulsesProduced;
                    }
                    if (actualPulsesProduced <= g_positionControllerX)
                    {
                        g_positionControllerX -= actualPulsesProduced;
                    }
                    else
                    {
                        positionControllerEmergencyStop_Unsafe();
                    }
                    break;
                }
                default:
                {
                    positionControllerEmergencyStop_Unsafe();
                    break;
                }
            }
        }

        setInterruptMask(oldInterruptMask);

        LL_TIM_ClearFlag_CC1(TIM5);
        LL_TIM_DisableIT_CC1(TIM5);
    }
}

void positionControllerStop_Unsafe()
{
    // stop counter (this will stop motor movement)
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_DisableCounter(TIM5);
    // wait 1 millisecond for falling edge to move the motor
    // (might need to reduce this wait time in the future)
    // at the moment it's not a big deal as I need to query sensor
    // not doing any complicated movement anyway
    LL_mDelay(1);
    // disable motor controller
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
    setPositionControllerMovingLedEnabled(false);
}

void positionControllerEmergencyStop_Unsafe()
{
    // disable controller first
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
    // disable timer after that
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_DisableCounter(TIM5);
    // turn on emergency stop LED
    setEmergencyStopLedEnabled(true);
    // modify memory at the end of the method in case it generates some faults
    g_positionControllerXState = PCS_EMERGENCY_STOPPED;
    setPositionControllerMovingLedEnabled(false);
}

void positionControllerMoveUntilLimitSwitchTriggers_Unsafe(positionControllerDirection_t direction)
{
    if (direction == PCD_FORWARD)
    {
        // set forward direction
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
    }
    else if (direction == PCD_BACKWARD)
    {
        // set backward direction
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
    }
    else
    {
        return;
    }

    g_positionControllerXDirection = direction;

    // reset counter to 0 so we can count number of steps motor moves
    LL_TIM_SetCounter(TIM5, 0);
    // get ready to count TIM2 pulses
    LL_TIM_EnableCounter(TIM5);

    // enable motor controller
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
    // start moving motor
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
}

void positionControllerMove_Unsafe(positionControllerDirection_t direction, uint32_t pulseCount)
{
    if (direction == PCD_FORWARD)
    {
        // set forward direction
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
    }
    else if (direction == PCD_BACKWARD)
    {
        // set backward direction
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
    }
    else
    {
        return;
    }

    g_positionControllerXDirection = direction;
    g_positionControllerXPlannedPulseCount = pulseCount;

    // reset counter to 0 so we can count number of steps motor moves
    LL_TIM_SetCounter(TIM5, 0);

    LL_TIM_ClearFlag_CC1(TIM5);
    // enable interrupt to handle stop event
    LL_TIM_EnableIT_CC1(TIM5);

    // call interrupt (which stops motor) after pulseCount
    LL_TIM_OC_SetCompareCH1(TIM5, pulseCount);

    // get ready to count TIM2 pulses
    LL_TIM_EnableCounter(TIM5);

    // enable motor controller
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
    // start moving motor
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
}

void DMA1_Channel2_IRQHandler()
{
    if(LL_DMA_IsActiveFlag_TC2(DMA1) == 1)
    {
        // TODO
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        positionControllerEmergencyStop_Unsafe();
    }
    else if(LL_DMA_IsActiveFlag_TE2(DMA1) == 1)
    {
        // TODO
    }
}
