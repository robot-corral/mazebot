/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      executeTasksOnPsp
    .section    .text.executeTasksOnPsp
    .type       executeTasksOnPsp, %function

    .globl      executeTasks
    .section    .text.executeTasks
    .type       executeTasks, %function
executeTasks:
    # load CONTROL register to R3
    MRS         R3, CONTROL
    # set bit 1 (SPSEL = 1, this will make sure we use process stack instead of main stack)
    ORR         R3, #0x02
    # write R3 register into CONTROL register
    MSR         CONTROL, R3
    # write R0 which has process stack start address to SP (stack register)
    MSR         PSP, R0
executeTasksOnPsp:
    # call OS to schedule next task
    SVC         #0
    # this function never exits as SVC #0 will reset next instruction back to SVC #0
    .size       executeTasks, .-executeTasks

.end
