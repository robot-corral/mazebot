/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "../src/svc_defs.h"
#include "../inc/result_codes.h"
#include "../src/schedule_tasks_defs.h"

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      SVC_Handler
    .section    .text.SVC_Handler
    .type       SVC_Handler, %function
SVC_Handler:
    # store LR register as it is going to be changed by calling SVC handlers
    # need to use register R4 to store result address
    PUSH        {R4, LR}
    # as per spec ARM®v7-M Architecture Reference Manual
    # B1.5.8 Exception return behavior
    # 2nd bit (starting from zero) is:
    # - 0 if return stack is main (MSP)
    # - 1 if return stack is process (PSP)
    TST         LR, 4
    # true is when bit 2 is zero
    # this instruction determines how next 3 instructions are going to be executed
    # in this case it is: {then}, {then}, {else}
    # so 1st two are only executed if bit 2 is zero
    # last 3rd instruction is only executed if bit 2 is one
    ITTE        EQ
    # load MSP (main stack register) into R4
    MRSEQ       R4, MSP
    # add 2 * 4 bytes to R4 (this is to account for PUSH above which is done on main stack)
    ADDEQ       R4, 8
    # load PSP (process stack register) into R4
    MRSNE       R4, PSP
    # load PC value from caller stack to R14 so we can read previous instruction's SVC code
    # (previous instruction is SVS #imm where #imm is 8 bit value)
    LDR         R14, [R4, 24]
    # read previous instruction #imm argument value (into R14)
    LDRB        R14, [R14, -2]
    # compare R14 with SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK
    CMP         R14, SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK
    # R14 equals SVC SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK then go to labelFinishCurrentAndMoveToNextTaskSvc
    BEQ         labelFinishCurrentAndMoveToNextTaskSvc
    # comare R14 with SVC_SCHEDULE_TASK
    CMP         R14, SVC_SCHEDULE_TASK
    # R14 equals SVC_SCHEDULE_TASK then go to labelScheduleTaskSvc
    BEQ         labelScheduleTaskSvc
    # store R_ERR_UNKNOWN_SVC_CALL result to be returned to the caller
    MOV         R0, R_ERR_UNKNOWN_SVC_CALL
    # go to end of svc call handling
    B           labelEndFunctionWithResult

labelFinishCurrentAndMoveToNextTaskSvc:
    # allocate memory for return parameters 3 * 4 = 12 bytes from finishCurrentAndMoveToNextTaskSvc function
    SUB         SP, 12
    # save address of 1st output parameter to R0
    MOV         R0, SP
    # save address of 2nd output parameter to R1
    ADD         R1, SP, 4
    # save address of 3rd output parameter to R2
    ADD         R2, SP, 8
    # call SVC C function
    BL          finishCurrentAndMoveToNextTaskSvc
    # compare result to 0
    TST         R0, 0
    # less than or equal to 0 then execute next 2 instructions
    ITT         LE
    # roll back stack by 3 * 4 = 12 bytes
    ADDLE       SP, 12
    # go to the end as error occured and we are done
    BLE         labelError
    # compare result stored in R0 to R_RESUME_NEW_TASK
    CMP         R0, R_RESUME_NEW_TASK
    # restore output parameters from stack to R0, R1, R2
    POP         {R0-R2}
    # R14 not equal to 3 go to labelStartNewTask
    BNE         labelStartNewTask
    # call resume task
    BL          resumeTask
    # go to the end
    B           labelEndFunction
labelStartNewTask:
    # call start new task
    BL          starkNewTask
    # go to the end
    B           labelEndFunction

labelScheduleTaskSvc:
    # call SVC C function
    BL          scheduleTaskSvc

labelEndFunctionWithResult:
    # store SVC OS call result on the stack (where R0 is stored) so it can be returned to calling function
    STR         R0, [R4]
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    POP         {R4, PC}

labelError:
    # repeat forever
    B           labelError

labelEndFunction:
    # restore remaining registers we used from the stack
    # to return from exception restore PC value from the stack
    # R4 was modified by task manager to contain executing task data so restore R12 which is caller saved register
    POP         {R12, PC}
    .size       SVC_Handler, .-SVC_Handler

.end
