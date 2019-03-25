/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "../src/svc_defs.h"

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      suspend
    .section    .text.suspend
    .type       suspend, %function
suspend:
    # perform SVC call (SVC_SUSPEND is a hardcoded literal)
    # result of the call is stored in R0
    SVC         SVC_SUSPEND
    # return to calling function
    BX          LR
    .size       suspend, .-suspend

.end
