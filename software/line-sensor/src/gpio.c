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

    // ports A/B/C have special unused pins which are going to retain their original reset configuration

    /*******************************************************************************
     * GPIOA
     *******************************************************************************/

    GPIOA->MODER = GPIO_MODE_00(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_01(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_02(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_03(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_04(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_05(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_06(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_07(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_08(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_09(GPIO_MODE_ANALOG) |    // UNUSED (ST_LINK_TX)
                   GPIO_MODE_10(GPIO_MODE_ANALOG) |    // UNUSED (ST_LINK_RX)
                   GPIO_MODE_11(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_12(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_13(GPIO_MODE_ALTERNATE) | // UNUSED (ST_LINK_JTMS_SWDIO)
                   GPIO_MODE_14(GPIO_MODE_ALTERNATE) | // UNUSED (ST_LINK_JTCK_SWCLK)
                   GPIO_MODE_15(GPIO_MODE_ALTERNATE);  // UNUSED (ST_LINK_JTDI)

    /*******************************************************************************
     * GPIOB
     *******************************************************************************/

    GPIOB->MODER = GPIO_MODE_00(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_01(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_02(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_03(GPIO_MODE_ALTERNATE) | // UNUSED (ST_LINK_JTDO_TRACESWO)
                   GPIO_MODE_04(GPIO_MODE_ALTERNATE) | // UNUSED (SYS_JTRST)
                   GPIO_MODE_05(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_06(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_07(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_08(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_09(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_10(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_11(GPIO_MODE_ANALOG) |    // UNUSED
                   GPIO_MODE_12(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_13(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_14(GPIO_MODE_ANALOG) |    // ADC
                   GPIO_MODE_15(GPIO_MODE_ANALOG);     // ADC

    /*******************************************************************************
     * GPIOC
     *******************************************************************************/

    GPIOC->MODER = GPIO_MODE_00(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_01(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_02(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_03(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_04(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_05(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_06(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_07(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_08(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_09(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_10(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_11(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_12(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_13(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_14(GPIO_MODE_INPUT)  | // UNUSED (RCC_OSC32_IN)
                   GPIO_MODE_15(GPIO_MODE_INPUT);   // UNUSED (RCC_OSC32_OUT)

    /*******************************************************************************
     * GPIOD
     *******************************************************************************/

    GPIOD->MODER = GPIO_MODE_00(GPIO_MODE_ALTERNATE) | // SPI2_NSS
                   GPIO_MODE_01(GPIO_MODE_ALTERNATE) | // SPI2_SCK
                   GPIO_MODE_02(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_03(GPIO_MODE_ALTERNATE) | // SPI2_MISO
                   GPIO_MODE_04(GPIO_MODE_ALTERNATE) | // SPI2_MOSI
                   GPIO_MODE_05(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_06(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_07(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_08(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_09(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_10(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_11(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_12(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_13(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_14(GPIO_MODE_ANALOG)    | // UNUSED
                   GPIO_MODE_15(GPIO_MODE_ANALOG);     // UNUSED

    GPIOD->OSPEEDR = GPIO_OSPEED_00(GPIO_OSPEED_VERY_HIGH_SPEED) | // SPI2_NSS
                     GPIO_OSPEED_01(GPIO_OSPEED_VERY_HIGH_SPEED) | // SPI2_SCK
                     GPIO_OSPEED_02(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_03(GPIO_OSPEED_VERY_HIGH_SPEED) | // SPI2_MISO
                     GPIO_OSPEED_04(GPIO_OSPEED_VERY_HIGH_SPEED) | // SPI2_MOSI
                     GPIO_OSPEED_05(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_06(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_07(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_08(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_09(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_10(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_11(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_12(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_13(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_14(GPIO_OSPEED_LOW_SPEED)       | // UNUSED
                     GPIO_OSPEED_15(GPIO_OSPEED_LOW_SPEED);        // UNUSED
    
    GPIOD->AFR[0] = GPIO_AF_00(GPIO_AF05) | // SPI2_NSS
                    GPIO_AF_01(GPIO_AF05) | // SPI2_SCK
                    GPIO_AF_02(GPIO_AF00) | // UNUSED
                    GPIO_AF_03(GPIO_AF05) | // SPI2_MISO
                    GPIO_AF_04(GPIO_AF05) | // SPI2_MOSI
                    GPIO_AF_05(GPIO_AF00) | // UNUSED
                    GPIO_AF_06(GPIO_AF00) | // UNUSED
                    GPIO_AF_07(GPIO_AF00);  // UNUSED

    /*******************************************************************************
     * GPIOE
     *******************************************************************************/

    GPIOE->MODER = GPIO_MODE_00(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_01(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_02(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_03(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_04(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_05(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_06(GPIO_MODE_ANALOG) | // SYS_WKUP3
                   GPIO_MODE_07(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_08(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_09(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_10(GPIO_MODE_ANALOG) | // ADC
                   GPIO_MODE_11(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_12(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_13(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_14(GPIO_MODE_ANALOG) | // UNUSED
                   GPIO_MODE_15(GPIO_MODE_ANALOG);  // UNUSED
}
