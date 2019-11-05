/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

#include <stdbool.h>

#define GPIO_MODE_INPUT     0b00
#define GPIO_MODE_OUTPUT    0b01
#define GPIO_MODE_ALTERNATE 0b10
#define GPIO_MODE_ANALOG    0b11

#define GPIO_MODE_00(mode) (mode)
#define GPIO_MODE_01(mode) (mode << 2)
#define GPIO_MODE_02(mode) (mode << 4)
#define GPIO_MODE_03(mode) (mode << 6)
#define GPIO_MODE_04(mode) (mode << 8)
#define GPIO_MODE_05(mode) (mode << 10)
#define GPIO_MODE_06(mode) (mode << 12)
#define GPIO_MODE_07(mode) (mode << 14)
#define GPIO_MODE_08(mode) (mode << 16)
#define GPIO_MODE_09(mode) (mode << 18)
#define GPIO_MODE_10(mode) (mode << 20)
#define GPIO_MODE_11(mode) (mode << 22)
#define GPIO_MODE_12(mode) (mode << 24)
#define GPIO_MODE_13(mode) (mode << 26)
#define GPIO_MODE_14(mode) (mode << 28)
#define GPIO_MODE_15(mode) (mode << 30)

#define GPIO_OSPEED_LOW_SPEED       0b00
#define GPIO_OSPEED_MEDIUM_SPEED    0b01
#define GPIO_OSPEED_HIGH_SPEED      0b10
#define GPIO_OSPEED_VERY_HIGH_SPEED 0b11

#define GPIO_OSPEED_00(ospeed) (ospeed)
#define GPIO_OSPEED_01(ospeed) (ospeed << 2)
#define GPIO_OSPEED_02(ospeed) (ospeed << 4)
#define GPIO_OSPEED_03(ospeed) (ospeed << 6)
#define GPIO_OSPEED_04(ospeed) (ospeed << 8)
#define GPIO_OSPEED_05(ospeed) (ospeed << 10)
#define GPIO_OSPEED_06(ospeed) (ospeed << 12)
#define GPIO_OSPEED_07(ospeed) (ospeed << 14)
#define GPIO_OSPEED_08(ospeed) (ospeed << 16)
#define GPIO_OSPEED_09(ospeed) (ospeed << 18)
#define GPIO_OSPEED_10(ospeed) (ospeed << 20)
#define GPIO_OSPEED_11(ospeed) (ospeed << 22)
#define GPIO_OSPEED_12(ospeed) (ospeed << 24)
#define GPIO_OSPEED_13(ospeed) (ospeed << 26)
#define GPIO_OSPEED_14(ospeed) (ospeed << 28)
#define GPIO_OSPEED_15(ospeed) (ospeed << 30)

#define GPIO_AF00 0b0000
#define GPIO_AF01 0b0001
#define GPIO_AF02 0b0010
#define GPIO_AF03 0b0011
#define GPIO_AF04 0b0100
#define GPIO_AF05 0b0101
#define GPIO_AF06 0b0110
#define GPIO_AF07 0b0111
#define GPIO_AF08 0b1000
#define GPIO_AF09 0b1001
#define GPIO_AF10 0b1010
#define GPIO_AF11 0b1011
#define GPIO_AF12 0b1100
#define GPIO_AF13 0b1101
#define GPIO_AF14 0b1110
#define GPIO_AF15 0b1111

#define GPIO_AF_00(af) (af)
#define GPIO_AF_01(af) (af << 4)
#define GPIO_AF_02(af) (af << 8)
#define GPIO_AF_03(af) (af << 12)
#define GPIO_AF_04(af) (af << 16)
#define GPIO_AF_05(af) (af << 20)
#define GPIO_AF_06(af) (af << 24)
#define GPIO_AF_07(af) (af << 28)

/*
 * initializes all GPIO pins as fast as possible to decrease startup time.
 * should only be called once after reset.
 */
void initializeGpio();
