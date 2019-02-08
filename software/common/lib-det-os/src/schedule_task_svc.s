/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

.syntax unified
    .cpu cortex-m4
    .fpu vfpv2
    .thumb

    .globl      scheduleTask
    .section    .text.scheduleTask
    .type       scheduleTask, %function
scheduleTask:
    # store R0 and return address on the stack
    # R0 will hold return value from SVC call once SVC call is complete
    PUSH        {R0, LR}
    # perform SVC call
    SVC         #1
    # restore registers and return to the caller
    POP         {R0, PC}
    .size       scheduleTask, .-scheduleTask

.end
