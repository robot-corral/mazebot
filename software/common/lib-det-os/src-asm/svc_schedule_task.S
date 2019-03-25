/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "../src/svc_defs.h"

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      scheduleTask
    .section    .text.scheduleTask
    .type       scheduleTask, %function
scheduleTask:
    # perform SVC call (SVC_SCHEDULE_TASK is a hardcoded literal)
    # result of the call is stored in R0
    SVC         SVC_SCHEDULE_TASK
    # return to calling function
    BX          LR
    .size       scheduleTask, .-scheduleTask

.end
