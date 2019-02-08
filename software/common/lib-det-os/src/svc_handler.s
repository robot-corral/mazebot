/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

# TODO how to handle parameters which are on stack (for functions which use more than just R0-R4)

.syntax unified
    .cpu cortex-m4
    .fpu vfpv2
    .thumb

    .globl      SVC_Handler
    .section    .text.SVC_Handler
    .type       SVC_Handler, %function
SVC_Handler:
    # exception handler on ARM stores R3, R2, R1, R0 registers so we don't need to store them
    # store R4, R5, R6 registers as we are going to change them in our SVC_Handler
    # store return address
    # (stack must be 8 bytes aligned so we need to store even number of registers)
    PUSH        {R4, R5, R6, LR}
    # as per spec ARM®v7-M Architecture Reference Manual
    # B1.5.8 Exception return behavior
    # 2nd bit (starting from zero) is:
    # - zero if return stack is main (MSP)
    # - one if return stack is process (PSP)
    TST         LR, #4
    ITE         NE
    MRSNE       R4, PSP
    MRSEQ       R4, MSP
    # load PC value so we can read previous instruction's SVC code
    # (previous instruction is SVS #imm where #imm is 8 bit value)
    LDR         R5, [R4, #40]
    # read previous instruction #imm argument value (into R6)
    LDRB        R6, [R5, #-2]
#
# TODO need to add branching to handle different SVC calls
#
    # call C function to process SVC OS call
    BL          scheduleTaskSvc
    # store SVC OS call result on the stack so it can be returned from original function
    STR         R0, [R4, #48]
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    POP         {R4, R5, R6, PC}
    .size       SVC_Handler, .-SVC_Handler

.end
