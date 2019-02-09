/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

 /*
  * @param R0 void*     pTaskParameter
  * @param R1 task_t    task
  * @param R2 void*     pStackStartAddress
  */

.syntax unified
    .cpu cortex-m4
    .fpu vfpv2
    .thumb

    .globl      executeTask
    .section    .text.executeTask
    .type       executeTask, %function
executeTask:
    # store R4 register on stack as we are going to modify it
    # store return address on stack
    PUSH        {R4, LR}
    # load CONTROL register to R4
    MRS         R4, CONTROL
    # set bit 1 (SPSEL = 1, this will make sure we use process stack instead of main stack)
    ORR         R4, #0x02
    # write R4 register into CONTROL register
    MSR         CONTROL, R4
    # write R2 which has process stack start address to SP (stack register)
    MOV         SP, R2
    # call task function
    # R0 is a parameter passed into task function
    BLX         R1
    # load CONTROL register to R4
    MRS         R4, CONTROL
    # clear bit 1 (SPSEL = 0, this will make sure we use main stack)
    AND         R4, #0xFFFFFFFD
    # write R4 register to CONTROL register
    MSR         CONTROL, R4
    # restore changed registers and return
    POP         {R4, PC}
    .size       executeTask, .-executeTask

.end
