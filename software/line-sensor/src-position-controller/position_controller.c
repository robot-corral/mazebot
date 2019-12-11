#include "position_controller.h"

#include "arm.h"
#include "led.h"
#include "global_data.h"
#include "interrupt_priorities.h"

#include <stdatomic.h>

#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_gpio.h>
#include <stm32\stm32l4xx_ll_utils.h>

static void initializeSlaveTimer();
static void initializeMasterTimer();
static void positionControllerStop();
static bool positionControllerMove(positionControllerStatus_t desired);

void initializePositionController()
{
    g_positionControllerX = 0;
    g_positionControllerXMaxValue = 0;
    g_positionControllerXStatus = PCS_FREE;
    g_positionControllerXDesiredValue = 0;
    g_positionControllerXDirection = PCD_NONE;
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

    LL_TIM_SetAutoReload(TIM5, 1);
    LL_TIM_SetCounter(TIM5, 1);

    LL_TIM_EnableIT_UPDATE(TIM5);
}

void initializeMasterTimer()
{
    const uint32_t prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 1000000);

    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetPrescaler(TIM2, prescaler);
    LL_TIM_EnableARRPreload(TIM2);

    const uint32_t cycle = __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM2), 1500);

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

bool isPositionControllerBusy()
{
    return atomic_load(&g_positionControllerXStatus) != PCS_FREE;
}

uint32_t getPosition()
{
    return g_positionControllerX;
}

bool positionControllerMove(positionControllerStatus_t desired)
{
    bool result = false;
    // in case emergency stop happened we don't want to resume moving the motor
    // so make sure we cannot get interrupted
    raise_interrupt_priority();
    if (atomic_load(&g_positionControllerXStatus) == desired)
    {
        setGreenLedEnabled(true);
        // get ready to count TIM2 pulses
        LL_TIM_EnableCounter(TIM5);
        // enable motor controller
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
        // start moving motor
        LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
        LL_TIM_EnableCounter(TIM2);
        LL_TIM_GenerateEvent_UPDATE(TIM2);
        result = true;
    }
    restore_interrupt_priority();
    return result;
}

bool setPosition(positionControllerDirection_t direction, uint32_t pulseCount)
{
    if (pulseCount == 0)
    {
        return true; // we are done!
    }

    positionControllerStatus_t desired;

    switch (direction)
    {
        case PCD_FORWARD  : desired = PCS_MOVING_FORWARD; break;
        case PCD_BACKWARD : desired = PCS_MOVING_BACKWARD; break;
        default           : return false;
    }

    positionControllerStatus_t expected = PCS_FREE;

    if (!atomic_compare_exchange_strong(&g_positionControllerXStatus, &expected, desired))
    {
        return false;
    }

    g_positionControllerXDirection = direction;

    const uint32_t positionControllerX = g_positionControllerX;

    switch (direction)
    {
        case PCD_FORWARD:
        {
            // make sure we don't trigger max limit-switch
            if (pulseCount > g_positionControllerXMaxValue ||
                positionControllerX > g_positionControllerXMaxValue - pulseCount)
            {
                // make sure we do not overwrite status if it changed since we set it to desired value
                expected = desired;
                atomic_compare_exchange_strong(&g_positionControllerXStatus, &expected, PCS_FREE);
                return false;
            }
            g_positionControllerXDesiredValue = positionControllerX + pulseCount;
            // set forward direction
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
            break;
        }
        case PCD_BACKWARD:
        {
            // make sure we don't trigger min limit-switch
            if (pulseCount > positionControllerX)
            {
                // make sure we do not overwrite status if it changed since we set it to desired value
                expected = desired;
                atomic_compare_exchange_strong(&g_positionControllerXStatus, &expected, PCS_FREE);
                return false;
            }
            g_positionControllerXDesiredValue = positionControllerX - pulseCount;
            // set backward direction
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
            break;
        }
        default:
        {
            // something went horribly wrong if we ended up here
            positionControllerEmergencyStop();
            return false;
        }
    }

    return positionControllerMove(desired);
}

void TIM5_IRQHandler()
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM5) == 1)
    {
        LL_TIM_ClearFlag_UPDATE(TIM5);

        bool stop = false;

        switch (g_positionControllerXDirection)
        {
            case PCD_FORWARD :
            {
                if (g_positionControllerX >= g_positionControllerXMaxValue)
                {
                    positionControllerEmergencyStop();
                    return;
                }
                g_positionControllerX = g_positionControllerX + 1;
                if (g_positionControllerX == g_positionControllerXDesiredValue)
                {
                    stop = true;
                }
                else if (g_positionControllerX > g_positionControllerXDesiredValue)
                {
                    positionControllerEmergencyStop();
                    return;
                }
                break;
            }
            case PCD_BACKWARD :
            {
                if (g_positionControllerX == 0)
                {
                    positionControllerEmergencyStop();
                    return;
                }
                g_positionControllerX = g_positionControllerX - 1;
                if (g_positionControllerX == g_positionControllerXDesiredValue)
                {
                    stop = true;
                }
                else if (g_positionControllerX < g_positionControllerXDesiredValue)
                {
                    positionControllerEmergencyStop();
                    return;
                }
                break;
            }
            default:
            {
                // something went horribly wrong if we ended up here
                positionControllerEmergencyStop();
                return;
            }
        }
        if (stop)
        {
            positionControllerStop();
            // now we are free to do whatever
            atomic_store(&g_positionControllerXStatus, PCS_FREE);
        }
    }
}

void NMI_Handler()
{
    positionControllerEmergencyStop();
    setBlueLedEnabled(true);
    for (;;) ;
}

void HardFault_Handler()
{
    positionControllerEmergencyStop();
    setBlueLedEnabled(true);
    for (;;) ;
}

void MemManage_Handler()
{
    positionControllerEmergencyStop();
    setBlueLedEnabled(true);
    for (;;) ;
}

void BusFault_Handler()
{
    positionControllerEmergencyStop();
    setBlueLedEnabled(true);
    for (;;) ;
}

/*
 * this method is called from highest priority interrupts
 */
void positionControllerEmergencyStop()
{
    // disable controller first
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
    // disable timer after that
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_DisableCounter(TIM5);
    // turn on emergency stop LED
    setRedLedEnabled(true);
    // modify memory at the end of the method in case it generates some faults
    atomic_store(&g_positionControllerXStatus, PCS_EMERGENCY_STOPPED);
}

void positionControllerStop()
{
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
    setGreenLedEnabled(false);
}

bool calibratePositionController()
{
    positionControllerStatus_t expected = PCS_FREE;

    if (!atomic_compare_exchange_strong(&g_positionControllerXStatus, &expected, PCS_CALIBRATING_MIN))
    {
        return false;
    }

    // move to min position 1st
    g_positionControllerX = UINT32_MAX;
    g_positionControllerXDesiredValue = 0;
    g_positionControllerXMaxValue = UINT32_MAX;
    g_positionControllerXDirection = PCD_BACKWARD;

    // set backward direction
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);

    return positionControllerMove(PCS_CALIBRATING_MIN);
}

/*
 * the only way to interrupt this method is with emergency stop interrupt
 */
void positionControllerLimitStop(positionControllerLimitStopType_t limitStopType)
{
    positionControllerStop();

    const positionControllerStatus_t positionControllerXStatus = atomic_load(&g_positionControllerXStatus);

    if (positionControllerXStatus == PCS_CALIBRATING_MIN && limitStopType == PCLST_MIN)
    {
        positionControllerStatus_t expected = PCS_CALIBRATING_MIN;
        if (atomic_compare_exchange_strong(&g_positionControllerXStatus, &expected, PCS_CALIBRATING_MAX))
        {
            // move to max position
            g_positionControllerX = 0;
            g_positionControllerXMaxValue = UINT32_MAX;
            g_positionControllerXDirection = PCD_FORWARD;
            g_positionControllerXDesiredValue = UINT32_MAX;

            // set forward direction
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);

            positionControllerMove(PCS_CALIBRATING_MAX);
        }
    }
    else if (positionControllerXStatus == PCS_CALIBRATING_MAX)
    {
        if (limitStopType == PCLST_MAX)
        {
            // we figured out max x value
            g_positionControllerXDesiredValue = 0;
            g_positionControllerXDirection = PCD_NONE;
            g_positionControllerXMaxValue = g_positionControllerX;

            // we can now use the position controller
            positionControllerStatus_t expected = PCS_CALIBRATING_MAX;
            atomic_compare_exchange_strong(&g_positionControllerXStatus, &expected, PCS_FREE);
        }
    }
    else
    {
        atomic_store(&g_positionControllerXStatus, PCS_EMERGENCY_STOPPED);
        setRedLedEnabled(true);
    }
}
