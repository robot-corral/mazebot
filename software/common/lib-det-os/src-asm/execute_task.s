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
    # need to store LR as we are going to call task functions and it will be changed
    # to keep stack 8 byte aligned we store dummy R4 register as well
    PUSH        {R4, LR}
executeNewTask:
    # load CONTROL register to R3
    MRS         R3, CONTROL
    # set bit 1 (SPSEL = 1, this will make sure we use process stack instead of main stack)
    ORR         R3, #0x02
    # write R3 register into CONTROL register
    MSR         CONTROL, R3
    # write R2 which has process stack start address to SP (stack register)
    MOV         SP, R2
    # call task function (address is in R1)
    # R0 has parameter which is passed into task function
    BLX         R1
    # load CONTROL register to R3
    MRS         R3, CONTROL
    # clear bit 1 (SPSEL = 0, this will make sure we use main stack)
    AND         R3, #0xFFFFFFFD
    # write R3 register to CONTROL register
    MSR         CONTROL, R3
    # call OS to schedule next task
    SVC         #0
    # check if result == R_OK
    CMP         R0, 0
    # if not we are done
    BNE         finishExecutingTasks
    # store pTaskParameter returned by SVC call to R0
    MOV         R0, R1
    # store task returned by SVC call to R1
    MOV         R1, R2
    # store pStackStartAddress returned by SVC call to R2
    MOV         R2, R3
    # go and execute new task
    B           executeNewTask
finishExecutingTasks:
    # restore LR register and return to caller
    POP         {R4, PC}
    .size       executeTask, .-executeTask

.end
