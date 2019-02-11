/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

/*
 * R0 - void*  pTaskParameter
 * R1 - task_t task
 * R2 - void*  pStackStartAddress
 */

/*
 * stack after 1st call to SVC #0:
 * 20000268 007a11ff 00000000 00000002 00000000 08000911 08000936 41000000
 * r0       r1       r2       r3       r12      lr       return   xPSR
 *                                                       address
 *                                                       ^^^^^^^ <- next instruction address after SVC #0 call
 *                                              ^^^^^^^^ <- next instruction after executeTasks(...) method
 * 
 * fix:
 * 
 * 1) replace LR with ReturnAddress - 2 (08000936 - 2) (to make sure that next instruction after task is SVC #0)
 * 2) replace ReturnAddress with address of the task to be executed
 * 3) replace R0 with the address of the parameter
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
    # load 2 registers in one instruction
    # load previous ReturnAddress to R0
    # load previous xPSR to R3
    LDRD        R0, R3, [R3, #24]
    # move previous ReturnAddress to previous instruction (SVC #0)
    SUB         R0, R0, #2
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
