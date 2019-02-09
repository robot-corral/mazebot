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
    # perform SVC call (#1 is a hardcoded literal)
    # result of the call is stored in R0
    SVC         #1
    # return to calling function
    BX          LR
    .size       scheduleTask, .-scheduleTask

.end
