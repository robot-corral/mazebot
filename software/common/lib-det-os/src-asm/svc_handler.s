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
    # store LR register as it is going to be changed by calling SVC handlers
    # to keep stack 8 byte aligned we also store dummy R4 register
    PUSH        {R4, LR}
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
    # last 3rd instruction is only executed if bit 2 is one
    ITTE        EQ
    # load MSP (main stack register) into R4
    MRSEQ       R4, MSP
    # add 2 * 4 bytes to R4 (this is to account for PUSH above which is done on main stack)
    ADDEQ       R4, #8
    # load PSP (process stack register) into R4
    MRSNE       R4, PSP
    # load PC value from caller stack so we can read previous instruction's SVC code
    # (previous instruction is SVS #imm where #imm is 8 bit value)
    LDR         R3, [R4, #24]
    # read previous instruction #imm argument value (into R3)
    LDRB        R3, [R3, #-2]
    # is it a SVC 0 handler?
    CMP         R3, 0
    # if so go to code which calls SVC 0 handler
    BEQ         callScheduleNextTaskSvc
    # is it a SVC 1 handler?
    CMP         R3, 1
    # if so go to code which calls SVC 1 handler
    BEQ         callScheduleTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

callScheduleNextTaskSvc:
    # push R1, R2, R3 onto stack (they will hold result from scheduleNextTaskSvc)
    # to keep stack 8 byte aligned we also add dummy R4 to it
    PUSH        {R1, R2, R3, R4}
    # R2 should have address of 3rd parameter
    ADD         R2, SP, #8
    # R1 should have address of 2nd parameter
    ADD         R1, SP, #4
    # R0 should have address of 1st parameter
    MOV         R0, SP
    # call SVC C function
    BL          scheduleNextTaskSvc
    # retrieve all stored registers from the stack
    POP         {R1, R2, R3, R4}
    # put 1st parameter onto caller stack (in place of R1 register)
    STR         R1, [R4, #4]
    # put 2nd parameter onto caller stack (in place of R2 register)
    STR         R2, [R4, #8]
    # put 3rd parameter onto caller stack (in place of R3 register)
    STR         R3, [R4, #12]
    # go to end of svc call handling
    B           endOfSvcHandling

callScheduleTaskSvc:
    # call SVC C function
    BL          scheduleTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

endOfSvcHandling:
    # store SVC OS call result on the stack (where R0 is stored) so it can be returned to calling function
    STR         R0, [R4]
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    POP         {R4, PC}
    .size       SVC_Handler, .-SVC_Handler

.end
