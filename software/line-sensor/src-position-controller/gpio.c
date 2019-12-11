#include "gpio.h"

#include <stm32\stm32l4xx_ll_gpio.h>

void intializeGpio()
{
    /***************************************************************************
     * Motor control pins
     ***************************************************************************/

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);

    // disable motor controller
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);

    /***************************************************************************
     * Emergency button
     ***************************************************************************/

    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_NO);

    /***************************************************************************
     * Limit switches
     ***************************************************************************/

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_1, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_1, LL_GPIO_PULL_NO);

    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_3, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOD, LL_GPIO_PIN_3, LL_GPIO_PULL_NO);

    /***************************************************************************
     * Motor control PWM timer output
     ***************************************************************************/

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_1);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);
    // do not pull up as we might issue unwanted pulses and move the motor
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_DOWN);

    /***************************************************************************
     * LED
     ***************************************************************************/

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_14);
    
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);

    /***************************************************************************
     * Client communication usart
     ***************************************************************************/

    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(GPIOG, LL_GPIO_PIN_7, LL_GPIO_AF_8);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOG, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOG, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);

    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_8_15(GPIOG, LL_GPIO_PIN_8, LL_GPIO_AF_8);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOG, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOG, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);
}
