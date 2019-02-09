/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      taskSchedulerTimerCallback
    .section    .text.taskSchedulerTimerCallback
    .type       taskSchedulerTimerCallback, %function
taskSchedulerTimerCallback:
    PUSH        {R0, LR}
    NOP
    POP         {R0, PC}
    .size       taskSchedulerTimerCallback, .-taskSchedulerTimerCallback

.end
