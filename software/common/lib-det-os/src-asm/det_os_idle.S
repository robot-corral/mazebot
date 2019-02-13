/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      detOsIdle
    .section    .text.detOsIdle
    .type       detOsIdle, %function
detOsIdle:
    B           detOsIdle
    .size       detOsIdle, .-detOsIdle

.end
