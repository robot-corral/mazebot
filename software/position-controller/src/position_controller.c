#include "position_controller.h"

#include "led.h"
#include "global_data.h"
#include "interrupt_priorities.h"
#include "position_controller_atomic.h"

static void initializeSlaveTimer();
static void initializeMasterTimer();
static void initializeMasterTimerDma();

/*
 * returns true if stop was performed and false if position controller is emergency stopped
 */
static positionControllerState_t positionControllerMoveUntilLimitSwitchTriggers(positionControllerDirection_t direction);
static positionControllerState_t positionControllerMove(positionControllerDirection_t direction, uint32_t pulseCount);

static void finishedRepositioning();

void initializePositionController()
{
    g_positionControllerX = 0;
    g_positionControllerXMaxValue = 0;
    g_positionControllerXPulseErrorCount = 0;
    g_positionControllerXPlannedPulseCount = 0;

    g_positionControllerXState = PCS_RESET;
    g_positionControllerXDirection = PCD_NONE;
    g_positionControllerCalibratingState = PCCS_NONE;

    initializeMasterTimerDma();
    initializeSlaveTimer();
    initializeMasterTimer();
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

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);
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

    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    // timer update is on falling edge
    // idle timer state is low
    // (as motor controller expects rising edge, then falling edge on which it moves the motor)
    LL_TIM_OC_ConfigOutput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH | LL_TIM_OCIDLESTATE_LOW);
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
    moveRequestResult_t result;
    positionControllerState_t positionControllerXState = g_positionControllerXState;

    if (positionControllerXState == PCS_RESET || positionControllerXState == PCS_IDLE)
    {
        // move to min position 1st
        result = MRR_OK;
        g_positionControllerCalibratingState = PCCS_CALIBRATING_MIN;
        positionControllerXState = positionControllerMoveUntilLimitSwitchTriggers(PCD_BACKWARD);
    }
    else if ((positionControllerXState & PCS_BUSY) == PCS_BUSY)
    {
        result = MRR_BUSY;
    }
    else
    {
        result = MRR_INVALID_STATE;
    }

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

    if (direction != PCD_FORWARD && direction != PCD_BACKWARD)
    {
        if (pState)
        {
            *pState = g_positionControllerXState;
        }
        return MRR_INVALID_PARAMETER;
    }

    moveRequestResult_t result = MRR_OK;
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
                    positionControllerXState = positionControllerMove(PCD_FORWARD, pulseCount);
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
                    positionControllerXState = positionControllerMove(PCD_BACKWARD, pulseCount);
                }
                break;
            }
            default:
            {
                // something went horribly wrong if we ended up here
                // (we did check direction argument at the beginning of the method)
                positionControllerEmergencyStop();
                positionControllerXState = PCS_EMERGENCY_STOPPED;
                result = MRR_UNEXPECTED_ERROR;
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
    // stop acceleration / deceleration dma
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    setPositionControllerMovingLedEnabled(false);

    setInterruptMask(oldInterruptMask);
}

positionControllerState_t positionControllerMoveUntilLimitSwitchTriggers(positionControllerDirection_t direction)
{
    // it is safe to update direction even if we are in emergency stopped situation as motor controller is disabled
    // and so is signal generation

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
        // direction is wrong so it's safe to perform emergency stop
        positionControllerEmergencyStop();
        return PCS_EMERGENCY_STOPPED;
    }

    g_positionControllerXDirection = direction;

    return positionControllerMoveWithAcceleration_Atomic(PCS_BUSY_ACCELERATING_AND_MOVING_AT_CONSTANT_SPEED, DMA_TIMER_STEPS_COUNT);
}

void positionControllerLimitStop(positionControllerLimitStopType_t limitStopType)
{
    bool performEmergencyStop = true;
    const positionControllerCalibratingState_t positionControllerCalibratingState = g_positionControllerCalibratingState;

    if (positionControllerCalibratingState == PCCS_CALIBRATING_MIN)
    {
        if (limitStopType == PCLST_MIN)
        {
            if (positionControllerStop_Atomic())
            {
                // restart counting from 0
                LL_TIM_SetCounter(TIM5, 0);
                // move to max position
                g_positionControllerCalibratingState = PCCS_CALIBRATING_MAX;
                positionControllerMoveUntilLimitSwitchTriggers(PCD_FORWARD);
                performEmergencyStop = false;
            }
        }
        else
        {
            // ignore max limit switch as we are moving away from it
            performEmergencyStop = false;
        }
    }
    else if (positionControllerCalibratingState == PCCS_CALIBRATING_MAX)
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
            if (positionControllerStop_Atomic())
            {
                // remember max position
                g_positionControllerXMaxValue = LL_TIM_GetCounter(TIM5);
                g_positionControllerX = g_positionControllerXMaxValue;
                g_positionControllerCalibratingState = PCCS_CALIBRATING_CORRECT_MAX;
                // move to the middle (this gives us opportunity to get away from
                // max limit switch and update max position if it triggers again
                positionControllerMove(PCD_BACKWARD, g_positionControllerXMaxValue >> 1);
                performEmergencyStop = false;
            }
        }
    }
    else if (positionControllerCalibratingState == PCCS_CALIBRATING_CORRECT_MAX)
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
        positionControllerEmergencyStop();
    }
}

void TIM5_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_CC1(TIM5) == 1)
    {
        if (g_positionControllerXState == PCS_BUSY_ACCELERATING_AND_MOVING_AT_CONSTANT_SPEED)
        {
            startSlowingDown_Atomic(g_slowDownPulseCount);
        }
        else if (g_positionControllerXState == PCS_BUSY_MOVING_AT_CONSTANT_SPEED)
        {
            finishedRepositioning();
        }
        else
        {
            positionControllerEmergencyStop();
        }

        LL_TIM_ClearFlag_CC1(TIM5);
        LL_TIM_DisableIT_CC1(TIM5);
    }
}

void DMA1_Channel2_IRQHandler()
{
    if (LL_DMA_IsActiveFlag_TE2(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TE2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        positionControllerEmergencyStop();
    }
    else if (LL_DMA_IsActiveFlag_TC2(DMA1) == 1)
    {
        LL_DMA_ClearFlag_TC2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

        const positionControllerState_t positionControllerXState = g_positionControllerXState;

        if (positionControllerXState == PCS_BUSY_SLOWING_DOWN)
        {
            // we just finished slowing down
            finishedRepositioning();
        }
        else if (positionControllerXState == PCS_BUSY_ACCELERATING)
        {
            // we just finished accelerating and now it's time to slow down
            startSlowingDown_Atomic(g_slowDownPulseCount);
        }
        else if (positionControllerXState == PCS_BUSY_ACCELERATING_AND_MOVING_AT_CONSTANT_SPEED)
        {
            // when accelerating or moving at a constant speed
            // control is done using TIM5 counter interrupt
        }
        else
        {
            // we weren't slowing down or moving so there must be some error
            positionControllerEmergencyStop();
        }
    }
}

positionControllerState_t positionControllerMove(positionControllerDirection_t direction, uint32_t pulseCount)
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
        // direction is wrong so it's safe to perform emergency stop
        positionControllerEmergencyStop();
        return PCS_EMERGENCY_STOPPED;
    }

    g_positionControllerXDirection = direction;
    g_positionControllerXPlannedPulseCount = pulseCount;

    if (pulseCount <= MAX_PULSES_AT_CONSTANT_SPEED)
    {
        // make sure we have no pending CC1 interrupt flags set
        LL_TIM_ClearFlag_CC1(TIM5);
        // enable interrupt to handle stop event
        LL_TIM_EnableIT_CC1(TIM5);
        // call interrupt after pulseCount
        LL_TIM_OC_SetCompareCH1(TIM5, pulseCount + 1);
        return positionControllerMoveAtConstantSpeed_Atomic(PCS_BUSY_MOVING_AT_CONSTANT_SPEED);
    }
    else if (pulseCount <= 2 * DMA_TIMER_STEPS_COUNT + 1)
    {
        const uint32_t accelerationPulseCount = (pulseCount + 1) / 2;
        g_slowDownPulseCount = pulseCount - accelerationPulseCount;
        return positionControllerMoveWithAcceleration_Atomic(PCS_BUSY_ACCELERATING, accelerationPulseCount - 1);
    }
    else
    {
        const uint32_t pulseCountBeforeSlowDown = pulseCount - DMA_TIMER_STEPS_COUNT;
        g_slowDownPulseCount = DMA_TIMER_STEPS_COUNT;
        // make sure we have no pending CC1 interrupt flags set
        LL_TIM_ClearFlag_CC1(TIM5);
        // enable interrupt to handle stop event
        LL_TIM_EnableIT_CC1(TIM5);
        // call interrupt after pulseCount
        LL_TIM_OC_SetCompareCH1(TIM5, pulseCountBeforeSlowDown);
        return positionControllerMoveWithAcceleration_Atomic(PCS_BUSY_ACCELERATING_AND_MOVING_AT_CONSTANT_SPEED, DMA_TIMER_STEPS_COUNT);
    }
}

void finishedRepositioning()
{
    const uint32_t actualPulsesProduced = LL_TIM_GetCounter(TIM5) - 1;

    if (!positionControllerStop_Atomic())
    {
        // emergency stop happened so we are done
        return;
    }

    const positionControllerState_t positionControllerXState = g_positionControllerXState;

    if (g_positionControllerCalibratingState == PCCS_CALIBRATING_CORRECT_MAX)
    {
        // we are done calibrating
        g_positionControllerCalibratingState = PCCS_NONE;
    }

    if (positionControllerXState == PCS_BUSY_SLOWING_DOWN ||
        positionControllerXState == PCS_BUSY_MOVING_AT_CONSTANT_SPEED)
    {
        if (g_positionControllerCalibratingState == PCCS_NONE)
        {
            // only change controller state if we are not calibrating anymore
            if (!setPositionControllerXState_Atomic(PCS_IDLE))
            {
                // emergency stop happened so we are done
                return;
            }
        }

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
                    positionControllerEmergencyStop();
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
                    positionControllerEmergencyStop();
                }
                break;
            }
            default:
            {
                positionControllerEmergencyStop();
                break;
            }
        }
    }
    else
    {
        positionControllerEmergencyStop();
    }
}
