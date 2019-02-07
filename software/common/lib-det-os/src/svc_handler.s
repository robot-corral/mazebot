/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

# TODO need to do branching once I have more than one system call
# TODO how to handle parameters which are on stack (for functions which use more than just R0-R4)

.syntax unified
    .cpu cortex-m4
    .fpu vfpv2
    .thumb

    .globl      SVC_Handler
    .section    .text.SVC_Handler
    .type       SVC_Handler, %function
SVC_Handler:
    # store R0 register on the stack as it is used to return result_t from [function]Svc handlers
    # store return address on the stack
    PUSH        {R0, LR}
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
    LDR         R5, [R4, #32]
    # read previous instruction #imm argument value
    LDRB        R0, [R5, #-2]
    # call C function to process SVC OS call
    BL          scheduleTaskSvc
    # store SVC OS call result on the stack so it can be returned from original function
    STR         R0, [R4, #40]
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    POP         {R0, PC}
    .size       SVC_Handler, .-SVC_Handler

.end
