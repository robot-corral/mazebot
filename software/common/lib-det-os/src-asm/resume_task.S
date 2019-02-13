/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

/*
 * R0 - void*  pStackStartAddress
 * R1 - void*  pRegisterStorage
 */

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      resumeTask
    .section    .text.resumeTask
    .type       resumeTask, %function
resumeTask:
    # load stack address from PSP to R3
    MRS         R3, PSP
    # load xPSR value from previous task PSP stack into R3
    LDR         R3, [R3, 28]
    # load suspended task register values and update R1 address to point to register value after R11
    LDM         R1!, {R4, R5, R6, R7, R8, R9, R10, R11}
    # load stack address into R2
    LDR         R2, [R1]
    # load stack address from R2 to PSP
    MSR         PSP, R2
    # save xPSR value from R3 to new task PSP stack
    STR         R3, [R2, 28]
    # return to calling function
    BX          LR
    .size       resumeTask, .-resumeTask

.end
