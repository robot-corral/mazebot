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
        // PA13 - UNUSED (ST_LINK_JTMS_SWDIO)
        // PA14 - UNUSED (ST_LINK_JTCK_SWCLK)
        // PA15 - UNUSED

        // pin             14  12  10  08  06  04  02  00
        GPIOA->MODER = 0b10101011111111111111111111111111;
        // pin           15  13  11  09  07  05  03  01
    }

    {
        // PB0  - ADC
        // PB1  - ADC
        // PB2  - ADC
        // PB3  - UNUSED (ST_LINK_JTDO_TRACESWO)
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

        // pin             14  12  10  08  06  04  02  00
        GPIOB->MODER = 0b11111111111111111111111010111111;
        // pin           15  13  11  09  07  05  03  01
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
        // PC14 - UNUSED (RCC_OSC32_IN)
        // PC15 - UNUSED (RCC_OSC32_OUT)

        // pin             14  12  10  08  06  04  02  00
        GPIOC->MODER = 0b00001111111111111111111111111111;
        // pin           15  13  11  09  07  05  03  01
    }

    {
        // PD0  - SPI2_NSS
        // PD1  - SPI2_SCK
        // PD2  - UNUSED
        // PD3  - SPI2_MISO
        // PD4  - SPI2_MOSI
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

        //
        // SPI2 pins
        //
        // mode               -   10 (alternate)
        // port pull up/down  -   00 (no)
        // output speed       -   10 (high <= 10 MHz)
        // output type        -    0 (push-pull)
        // alternate function - 0101 (AF5)

        // pin             14  12  10  08  06  04  02  00
        GPIOD->MODER = 0b11111111111111111111111010111010;
        // pin           15  13  11  09  07  05  03  01

        // pin               14  12  10  08  06  04  02  00
        GPIOD->OSPEEDR = 0b00000000000000000000001010001010;
        // pin             15  13  11  09  07  05  03  01

        // pin                0006    0004    0002    0000
        GPIOD->AFR[0] = 0b00000000000001010101000001010101;
        // pin            0007    0005    0003    0001
    }

    {
        // PE0  - UNUSED
        // PE1  - UNUSED
        // PE2  - UNUSED
        // PE3  - UNUSED
        // PE4  - UNUSED
        // PE5  - UNUSED
        // PE6  - SYS_WKUP3
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
