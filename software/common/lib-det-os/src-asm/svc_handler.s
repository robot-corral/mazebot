/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      SVC_Handler
    .section    .text.SVC_Handler
    .type       SVC_Handler, %function
SVC_Handler:
    # store LR register as it is going to be changed by calling SVC handlers
    # to keep stack 8 byte aligned need to store dummy register R12. R12 is chosen as, if we are going to resume a task we couldn't push/pop any registers apart from R0-R3 and R12,
    # R0-R3 are used to pass arguments in (so we couldn't modify them), so the only register left is R12.
    PUSH        {R12, LR}
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
    # load MSP (main stack register) into R12
    MRSEQ       R12, MSP
    # add 2 * 4 bytes to R12 (this is to account for PUSH above which is done on main stack)
    ADDEQ       R12, #8
    # load PSP (process stack register) into R12
    MRSNE       R12, PSP
    # load PC value from caller stack to R14 so we can read previous instruction's SVC code
    # (previous instruction is SVS #imm where #imm is 8 bit value)
    LDR         R14, [R12, #24]
    # read previous instruction #imm argument value (into R14)
    LDRB        R14, [R14, #-2]
    # is it a SVC 0 handler?
    CMP         R14, #0
    # if so go to code which calls SVC 0 handler
    BEQ         callMarkCurrentTaskAsCompletedAndStartNextTaskSvc
    # is it a SVC 1 handler?
    CMP         R14, #1
    # if so go to code which calls SVC 1 handler
    BEQ         callScheduleTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

callMarkCurrentTaskAsCompletedAndStartNextTaskSvc:
    # allocate memory for return parameters from moveToNextTaskSvc function
    PUSH        {R0-R2}
    # save address of 1st output parameter to R1
    MOV         R0, SP
    # save address of 2nd output parameter to R2
    ADD         R1, SP, 4
    # save address of 3rd output parameter to R3
    ADD         R2, SP, 8
    # call SVC C function
    BL          finishCurrentAndMoveToNextTaskSvc
    # compare result to 0
    TST         R0, #0
    # if less then 0 execute next 2 instructions
    ITT         MI
    # roll back stack by 3 * 4 = 12 bytes
    ADDMI       SP, #12
    # go to the end as error occured and we are done
    BMI         endOfSvcHandling
    # compare result stored in R0 to 3
    CMP         R0, #3
    # restore output parameters from stack to R0, R1, R2
    POP         {R0-R2}
    # if R14 not equal to 3 go to callStartNewTask
    BNE         callStartNewTask
    # call resume task
    BL          resumeTask
    # make sure result is R_OK = 0
    MOV         R0, 0
    # go to the end
    B           endOfSvcHandling
callStartNewTask:
    # call start new task
    BL          starkNewTask
    # make sure result is R_OK = 0
    MOV         R0, 0
    # go to the end
    B           endOfSvcHandling

callScheduleTaskSvc:
    # call SVC C function
    BL          scheduleTaskSvc
    # go to end of svc call handling
    B           endOfSvcHandling

endOfSvcHandling:
    # store SVC OS call result on the stack (where R0 is stored) so it can be returned to calling function
    STR         R0, [R12]
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    POP         {R12, PC}
    .size       SVC_Handler, .-SVC_Handler

.end
