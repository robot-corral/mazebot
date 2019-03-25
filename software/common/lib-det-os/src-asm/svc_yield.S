/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "../src/svc_defs.h"

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      yield
    .section    .text.yield
    .type       yield, %function
yield:
    # perform SVC call (SVC_YIELD is a hardcoded literal)
    # result of the call is stored in R0
    SVC         SVC_YIELD
    # return to calling function
    BX          LR
    .size       yield, .-yield

.end
