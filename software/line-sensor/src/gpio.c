#include "gpio.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_gpio.h>

void initializeGpio(communicationInterface_t communicationInterface)
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
        // PA13 - UNUSED (ST_LINK_SWD)
        // PA14 - UNUSED (ST_LINK_SWC)
        // PA15 - UNUSED

        // pin             14  12  10  08  06  04  02  00
        GPIOA->MODER = 0b10101011111111111111111111111111;
        // pin           15  13  11  09  07  05  03  01
    }

    {
        if (communicationInterface == CI_I2C)
        {
            // same as else block with the exception of:
            //
            // PB6 - I2C1_SCL
            // PB7 - I2C1_SDA
            //
            // for both pins:
            //
            // mode               -   10 (alternate)
            // port pull up/down  -   01 (pull up)
            // output speed       -   11 (very high)
            // output type        -    1 (open drain)
            // alternate function - 0100 (AF4)

            // pin               14  12  10  08  06  04  02  00
            GPIOB->MODER   = 0b11111111111111111010111010111111;
            // pin             15  13  11  09  07  05  03  01

            // pin               14  12  10  08  06  04  02  00
            GPIOB->PUPDR   = 0b00000000000000000101000100000000;
            // pin             15  13  11  09  07  05  03  01

            // pin               14  12  10  08  06  04  02  00
            GPIOB->OSPEEDR = 0b00000000000000001111000011000000;
            // pin             15  13  11  09  07  05  03  01

            // pin                              1 1 1 0 0 0 0 0
            //                                  4 2 0 8 6 4 2 0
            GPIOB->OTYPER  = 0b00000000000000000000000011000000;
            // pin                             1 1 1 0 0 0 0 0
            //                                 5 3 1 9 7 5 3 1

            // pin                 0006    0004    0002    0000
            GPIOB->AFR[0]  = 0b01000100000000000000000000000000;
            // pin             0007    0005    0003    0001
        }
        else
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

            // pin             14  12  10  08  06  04  02  00
            GPIOB->MODER = 0b11111111111111111111111010111111;
            // pin           15  13  11  09  07  05  03  01
        }
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
        GPIOC->MODER = 0b11111111111111111111111111111111;
    }

    {
        if (communicationInterface == CI_SPI)
        {
            // same as else block with the exception of:
            //
            // PD0 - SPI2_NSS
            // PD1 - SPI2_SCK
            // PD3 - SPI2_MISO
            // PD4 - SPI2_MOSI
            //
            // for all 4 pins:
            //
            // mode               -   10 (alternate)
            // port pull up/down  -   00 (no)
            // output speed       -   11 (very high)
            // output type        -    0 (push-pull)
            // alternate function - 0101 (AF5)

            // pin               14  12  10  08  06  04  02  00
            GPIOD->MODER   = 0b11111111110111111111111010111010;
            // pin             15  13  11  09  07  05  03  01

            // pin               14  12  10  08  06  04  02  00
            GPIOD->OSPEEDR = 0b00000000001100000000001111001111;
            // pin             15  13  11  09  07  05  03  01

            // pin                 0006    0004    0002    0000
            GPIOD->AFR[0]  = 0b00000000000001010101000001010101;
            // pin             0007    0005    0003    0001
        }
        else if (communicationInterface == CI_USART)
        {
            // same as else block with the exception of:
            //
            // PD8 - USART2_TX
            // PD9 - USART2_RX
            //
            // for both pins:
            //
            // mode               -   10 (alternate)
            // port pull up/down  -   00 (no)
            // output speed       -   11 (very high)
            // output type        -    0 (push-pull)
            // alternate function - 0111 (AF7)

            // pin               14  12  10  08  06  04  02  00
            GPIOD->MODER   = 0b11111111110110101111111111111111;
            // pin             15  13  11  09  07  05  03  01

            // pin               14  12  10  08  06  04  02  00
            GPIOD->OSPEEDR = 0b00000000001111110000000000000000;
            // pin             15  13  11  09  07  05  03  01

            // pin                 0014    0012    0010    0008
            GPIOD->AFR[1]  = 0b00000000000000000000000001110111;
            // pin             0015    0013    0011    0009
        }
        else
        {
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
            // PD10 - (output) Timing Pin 0
            // PD11 - UNUSED
            // PD12 - UNUSED
            // PD13 - UNUSED
            // PD14 - UNUSED
            // PD15 - UNUSED

            // for output pin:
            //
            // mode               -   01 (general purpose output mode)
            // port pull up/down  -   00 (no)
            // output speed       -   11 (very high)
            // output type        -    0 (push-pull)

            // pin               14  12  10  08  06  04  02  00
            GPIOD->MODER   = 0b11111111110111111111111111111111;
            // pin             15  13  11  09  07  05  03  01

            // pin               14  12  10  08  06  04  02  00
            GPIOD->OSPEEDR = 0b00000000001100000000000000000000;
            // pin             15  13  11  09  07  05  03  01
        }
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

    {
        // PF0  - UNUSED
        // PF1  - UNUSED
        // PF2  - UNUSED
        // PF3  - UNUSED
        // PF4  - UNUSED
        // PF5  - UNUSED
        // PF6  - ADC
        // PF7  - ADC
        // PF8  - ADC
        // PF9  - ADC
        // PF10 - ADC
        // PF11 - ADC
        // PF12 - ADC
        // PF13 - ADC
        // PF14 - ADC
        // PF15 - ADC
        GPIOF->MODER = 0b11111111111111111111111111111111;
    }

    {
        // PG0  - ADC
        // PG1  - ADC
        // PG2  - ADC
        // PG3  - ADC
        // PG4  - ADC
        // PG5  - UNUSED
        // PG6  - UNUSED
        // PG7  - UNUSED
        // PG8  - UNUSED
        // PG9  - UNUSED
        // PG10 - UNUSED (communication interface is only read on reset)
        // PG11 - UNUSED
        // PG12 - UNUSED (communication interface is only read on reset)
        // PG13 - UNUSED
        // PG14 - UNUSED
        // PG15 - UNUSED
        GPIOG->MODER = 0b11111111111111111111111111111111;
    }

    {
        // PH0  - UNUSED
        // PH1  - UNUSED
        // PH2  - UNUSED
        GPIOH->MODER = 0b00000000000000000000000000111111;
    }
}

communicationInterface_t getCommunicationInterface()
{
    // g port is by default:
    // mode         - 00 - input
    // pull-up/down - 00 - no
    // so no need to reconfigure pin 10/12 on reset and can simply read their values

    const uint32_t communicationInterfaceMask = LL_GPIO_PIN_10 | LL_GPIO_PIN_12;
    const uint32_t communicationInterface = LL_GPIO_IsInputPinSet(GPIOG, communicationInterfaceMask);

    if (communicationInterface == LL_GPIO_PIN_10)
    {
        return CI_I2C;
    }
    else if (communicationInterface == LL_GPIO_PIN_12)
    {
        return CI_SPI;
    }
    else if (communicationInterface == communicationInterfaceMask)
    {
        return CI_USART;
    }
    else
    {
        return CI_UNKNOWN;
    }
}

inline void setOutput0High()
{
    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_10);
}

inline void setOutput0Low()
{
    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_10);
}
