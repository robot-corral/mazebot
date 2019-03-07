/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "stm32/stm32l4xx_ll_gpio.h"

#include "gpio.h"
#include "driver.h"
#include "status.h"

void setCriticalError(Error_t error)
{
    shutdown();

    switch (error)
    {
        case ERROR_MOTOR: LL_GPIO_SetOutputPin(LED_ERROR_0_GPIO, LED_ERROR_0_PIN); break;
        case ERROR_DMA:   LL_GPIO_SetOutputPin(LED_ERROR_1_GPIO, LED_ERROR_1_PIN); break;
    }

    for (;;) ;
}

void setCalibrationStatus(CalibrationStatus_t calibrationStatus)
{
    switch (calibrationStatus)
    {
        case CS_OFF:
        {
            LL_GPIO_ResetOutputPin(LED_CALIBRATING_GPIO, LED_CALIBRATING_PIN);
            LL_GPIO_ResetOutputPin(LED_RUNNING_GPIO, LED_RUNNING_PIN);
            LL_GPIO_ResetOutputPin(LED_PAUSED_GPIO, LED_PAUSED_PIN);
            break;
        }
        case CS_CALIBRATING_WHITE:
        {
            LL_GPIO_SetOutputPin(LED_CALIBRATING_GPIO, LED_CALIBRATING_PIN);
            LL_GPIO_SetOutputPin(LED_RUNNING_GPIO, LED_RUNNING_PIN);
            LL_GPIO_ResetOutputPin(LED_PAUSED_GPIO, LED_PAUSED_PIN);
            break;
        }
        case CS_CALIBRATING_BLACK:
        {
            LL_GPIO_SetOutputPin(LED_CALIBRATING_GPIO, LED_CALIBRATING_PIN);
            LL_GPIO_ResetOutputPin(LED_RUNNING_GPIO, LED_RUNNING_PIN);
            LL_GPIO_SetOutputPin(LED_PAUSED_GPIO, LED_PAUSED_PIN);
            break;
        }
    }
}

void setRunningStatus(RunningStatus_t runningStatus)
{
    switch (runningStatus)
    {
        case RS_OFF:
        {
            LL_GPIO_ResetOutputPin(LED_RUNNING_GPIO, LED_RUNNING_PIN);
            LL_GPIO_ResetOutputPin(LED_PAUSED_GPIO, LED_PAUSED_PIN);
            break;
        }
        case RS_PAUSED:
        {
            LL_GPIO_ResetOutputPin(LED_RUNNING_GPIO, LED_RUNNING_PIN);
            LL_GPIO_SetOutputPin(LED_PAUSED_GPIO, LED_PAUSED_PIN);
            break;
        }
        case RS_RUNNING:
        {
            LL_GPIO_SetOutputPin(LED_RUNNING_GPIO, LED_RUNNING_PIN);
            LL_GPIO_ResetOutputPin(LED_PAUSED_GPIO, LED_PAUSED_PIN);
            break;
        }
    }
}
