/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "gpio.h"
#include "line_sensor_defs.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_gpio.h>

void initializeGpio()
{
    // all unused pins are set to analog mode to conserve power
    //
    // for analog (ADC) ports:
    //
    // mode               -   11 (analog)
    // port pull up/down  -   00 (no)

    // ports A/B have special unused pins which are going to retain their original reset configuration

    {
        // PA0  - ADC
        // PA1  - ADC
        // PA2  - ADC
        // PA3  - ADC
        // PA4  - ADC
        // PA5  - ADC
        // PA6  - ADC
        // PA7  - ADC
        // PA8  - UNUSED
        // PA9  - UNUSED (ST_LINK_TX)
        // PA10 - UNUSED (ST_LINK_RX)
        // PA11 - UNUSED
        // PA12 - UNUSED
        // PA13 - UNUSED (ST_LINK_TMS)
        // PA14 - UNUSED (ST_LINK_TCK)
        // PA15 - UNUSED

#ifndef ACTIVE_COMMUNICATION_SPI
        // pin             14  12  10  08  06  04  02  00
        GPIOA->MODER = 0b10101011111111111111111111111111;
        // pin           15  13  11  09  07  05  03  01
#else
        #error not implemented
#endif
    }

    {
        // PB0  - ADC
        // PB1  - ADC
        // PB2  - ADC
        // PB3  - UNUSED (ST_LINK_SWO)
        // PB4  - UNUSED (SYS_JTRST)
        // PB5  - UNUSED
        // PB6  - UNUSED
        // PB7  - UNUSED
        // PB8  - UNUSED
        // PB9  - UNUSED
        // PB10 - UNUSED
        // PB11 - UNUSED
        // PB12 - ADC
        // PB13 - ADC
        // PB14 - ADC
        // PB15 - ADC

#ifndef ACTIVE_COMMUNICATION_I2C
        // pin             14  12  10  08  06  04  02  00
        GPIOB->MODER = 0b11111111111111111111111010111111;
        // pin           15  13  11  09  07  05  03  01
#else
        #error not implemented
#endif
    }

    {
        // PC0  - ADC
        // PC1  - ADC
        // PC2  - ADC
        // PC3  - ADC
        // PC4  - ADC
        // PC5  - ADC
        // PC6  - UNUSED
        // PC7  - UNUSED
        // PC8  - UNUSED
        // PC9  - UNUSED
        // PC10 - UNUSED
        // PC11 - UNUSED
        // PC12 - UNUSED
        // PC13 - UNUSED
        // PC14 - UNUSED
        // PC15 - UNUSED

#ifndef ACTIVE_COMMUNICATION_SPI
        // pin             14  12  10  08  06  04  02  00
        GPIOC->MODER = 0b11111111111111111111111111111111;
        // pin           15  13  11  09  07  05  03  01
#else
        #error not implemented
#endif
    }

    {
#ifndef ACTIVE_COMMUNICATION_USART
        // PD0  - UNUSED
        // PD1  - UNUSED
        // PD2  - UNUSED
        // PD3  - UNUSED
        // PD4  - UNUSED
        // PD5  - UNUSED
        // PD6  - UNUSED
        // PD7  - UNUSED
        // PD8  - UNUSED
        // PD9  - UNUSED
        // PD10 - UNUSED
        // PD11 - UNUSED
        // PD12 - UNUSED
        // PD13 - UNUSED
        // PD14 - UNUSED
        // PD15 - UNUSED

        // pin             14  12  10  08  06  04  02  00
        GPIOC->MODER = 0b11111111111111111111111111111111;
        // pin           15  13  11  09  07  05  03  01
#else
        // same as else block with the exception of:
        //
        // PD6 - USART3_TX
        // PD5 - USART3_RX
        //
        // for both pins:
        //
        // mode               -   10 (alternate)
        // port pull up/down  -   00 (no)
        // output speed       -   11 (very high)
        // output type        -    0 (push-pull)
        // alternate function - 0111 (AF7)

        // pin             14  12  10  08  06  04  02  00
        GPIOD->MODER = 0b11111111111111111110101111111111;
        // pin           15  13  11  09  07  05  03  01

        // pin               14  12  10  08  06  04  02  00
        GPIOD->OSPEEDR = 0b00000000000000000011110000000000;
        // pin             15  13  11  09  07  05  03  01

        // pin                0006    0004    0002    0000
        GPIOD->AFR[0] = 0b00000111011100000000000000000000;
        // pin            0007    0005    0003    0001
#endif
    }

    {
        // PE0  - UNUSED
        // PE1  - UNUSED
        // PE2  - UNUSED
        // PE3  - UNUSED
        // PE4  - UNUSED
        // PE5  - UNUSED
        // PE6  - UNUSED
        // PE7  - ADC
        // PE8  - ADC
        // PE9  - ADC
        // PE10 - ADC
        // PE11 - UNUSED
        // PE12 - UNUSED
        // PE13 - UNUSED
        // PE14 - UNUSED
        // PE15 - UNUSED
        GPIOE->MODER = 0b11111111111111111111111111111111;
    }
}
