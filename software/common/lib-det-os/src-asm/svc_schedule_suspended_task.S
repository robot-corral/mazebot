/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "../src/svc_defs.h"

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      scheduleSuspendedTask
    .section    .text.scheduleSuspendedTask
    .type       scheduleSuspendedTask, %function
scheduleSuspendedTask:
    # perform SVC call (SVC_SCHEDULE_SUSPENDED_TASK is a hardcoded literal)
    # result of the call is stored in R0
    SVC         SVC_SCHEDULE_SUSPENDED_TASK
    # return to calling function
    BX          LR
    .size       scheduleSuspendedTask, .-scheduleSuspendedTask

.end
