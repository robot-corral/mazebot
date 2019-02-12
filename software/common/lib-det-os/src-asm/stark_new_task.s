/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

/*
 * R0 - void*  pTaskParameter
 * R1 - task_t task
 * R2 - void*  pStackStartAddress
 */

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      starkNewTask
    .section    .text.starkNewTask
    .type       starkNewTask, %function
starkNewTask:
    # load stack address from PSP to R3
    MRS         R3, PSP
    # occupy 8 words (4-byte) on the stack
    SUB         R2, #32
    # store stack address from R2 to PSP
    MSR         PSP, R2
    # save task parameter from R0 onto new process stack (at location where R0 is stored by exception handler)
    STR         R0, [R2]
    # load previous xPSR to R3
    LDR         R3, [R3, #28]
    # load address of SVC 0 to R0
    LDR         R0, =executeTasksOnPsp
    # make R2 point to LR on the new process stack
    ADD         R2, R2, #20
    # save 3 registers in one instruction
    # save previous ReturnAddress from R0 into LR on the new process stack
    # save task start address from R1 into ReturnAddress on the new process stack
    # save previous xPSR from R3 into xPSR on the new process stack
    STM         R2, {R0, R1, R3}
    # return to calling function
    BX          LR
    .size       starkNewTask, .-starkNewTask

.end
