/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

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
    # - 0 if return stack is main (MSP)
    # - 1 if return stack is process (PSP)
    TST         LR, #4
    # true is when bit 2 is zero
    # this instruction determines how next 3 instructions are going to be executed
    # in this case it is: {then}, {then}, {else}
    # so 1st two are only executed if bit 2 is zero
    # last 3rd instructions is only executed if bit 2 is one
    ITTE        EQ
    # load MSP (main stack register) into R4
    MRSEQ       R4, MSP
    # add 4 * 4 to R4 (this is to account for PUSH above which is done on main stack)
    ADDEQ       R4, #16
    # load PSP (process stack register) into R4
    MRSNE       R4, PSP
    # load PC value so we can read previous instruction's SVC code
    # (previous instruction is SVS #imm where #imm is 8 bit value)
    LDR         R5, [R4, #24]
    # read previous instruction #imm argument value (into R6)
    LDRB        R6, [R5, #-2]
    # is it a SVC 0 handler?
    CMP         R6, 0
    # if so go to code which calls SVC 0 handler
    BEQ         callScheduleNextTaskSvc
    # is it a SVC 1 handler?
    CMP         R6, 1
    # if so go to code which calls SVC 1 handler
    BEQ         callScheduleTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

callScheduleNextTaskSvc:
    # call SVC C function
    BL          scheduleNextTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

callScheduleTaskSvc:
    # call SVC C function
    BL          scheduleTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

endOfSvcHandling:

    # store SVC OS call result on the stack so it can be returned from original function
    STR         R0, [R4, #32]
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    POP         {R4, R5, R6, PC}
    .size       SVC_Handler, .-SVC_Handler

.end
