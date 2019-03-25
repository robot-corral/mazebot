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
    # load PC value from caller stack to R14 so we can read calling instruction's SVC code
    # (calling instruction is "SVS #imm" where #imm is 8 bit value)
    LDR         R14, [R4, 24]
    # read calling SVC instruction #imm argument value (into R14)
    LDRB        R14, [R14, -2]

    # compare R14 with SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK
    CMP         R14, SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK
    # R14 equals SVC SVC_FINISH_CURRENT_AND_MOVE_TO_NEXT_TASK then go to labelFinishCurrentAndMoveToNextTaskSvc
    BEQ         labelFinishCurrentAndMoveToNextTaskSvc

    # compare R14 with SVC_YIELD
    CMP         R14, SVC_YIELD
    # R14 equals SVC SVC_YIELD then go to labelCurrentTaskYieldedSvc
    BEQ         labelCurrentTaskYieldedSvc

    # compare R14 with SVC_SUSPEND
    CMP         R14, SVC_SUSPEND
    # R14 equals SVC SVC_SUSPEND then go to labelSuspendTaskSvc
    BEQ         labelSuspendTaskSvc

    # comare R14 with SVC_SCHEDULE_TASK
    CMP         R14, SVC_SCHEDULE_TASK
    # R14 equals SVC_SCHEDULE_TASK then go to labelScheduleTaskSvc
    BEQ         labelScheduleTaskSvc

    # comare R14 with SVC_SCHEDULE_SUSPENDED_TASK
    CMP         R14, SVC_SCHEDULE_SUSPENDED_TASK
    # R14 equals SVC_SCHEDULE_SUSPENDED_TASK then go to labelScheduleSuspendedTaskSvc
    BEQ         labelScheduleSuspendedTaskSvc

    # store R_ERR_UNKNOWN_SVC_CALL result to be returned to the caller
    MOV         R0, R_ERR_UNKNOWN_SVC_CALL
    # go to end of svc call handling
    B           labelEndFunctionWithResult

################################################################################
# svc calls which immediately affect tasks execution order
################################################################################

labelFinishCurrentAndMoveToNextTaskSvc:
    # allocate memory for return parameters 4 * 4 = 16 bytes from finishCurrentAndMoveToNextTaskSvc function
    SUB         SP, 16
    # save address of 1st output parameter to R0
    MOV         R0, SP
    # save address of 2nd output parameter to R1
    ADD         R1, SP, 4
    # save address of 3rd output parameter to R2
    ADD         R2, SP, 8
    # save address of 3rd output parameter to R3
    ADD         R3, SP, 12
    # call SVC C function
    BL          finishCurrentAndMoveToNextTaskSvc
    # suspend / start next task / resume next task
    BL          PendSV_Handler_ProcessTaskSchedulerResult
    # go to the end
    B           labelEndFunctionWhichDoesNotModityCalleSavedRegisters

labelCurrentTaskYieldedSvc:
    # allocate memory for return parameters 4 * 4 = 16 bytes from finishCurrentAndMoveToNextTaskSvc function
    SUB         SP, 16
    # save address of 1st output parameter to R0
    MOV         R0, SP
    # save address of 2nd output parameter to R1
    ADD         R1, SP, 4
    # save address of 3rd output parameter to R2
    ADD         R2, SP, 8
    # save address of 3rd output parameter to R3
    ADD         R3, SP, 12
    # call SVC C function
    BL          currentTaskYieldsAndMoveToNextTaskSvc
    # suspend / start next task / resume next task
    BL          PendSV_Handler_ProcessTaskSchedulerResult
    # go to the end
    B           labelEndFunctionWhichDoesNotModityCalleSavedRegisters

labelSuspendTaskSvc:
    # allocate memory for return parameters 3 * 4 = 12 bytes from delayCurrentAndMoveToNextTaskSvc function
    # we are going to allocate 4 more bytes in the push instruction below, so total storage size is 16 bytes
    SUB         SP, 12
    # store suspendDuration (which is in R0) onto a stack
    PUSH        {R0}
    # save address of 1st output parameter to R0
    MOV         R0, SP
    # save address of 2nd output parameter to R1
    ADD         R1, SP, 4
    # save address of 3rd output parameter to R2
    ADD         R2, SP, 8
    # save address of 3rd output parameter to R3
    ADD         R3, SP, 12
    # call SVC C function
    BL          delayCurrentAndMoveToNextTaskSvc
    # suspend / start next task / resume next task
    BL          PendSV_Handler_ProcessTaskSchedulerResult
    # go to the end
    B           labelEndFunctionWhichDoesNotModityCalleSavedRegisters

labelEndFunctionWhichDoesNotModityCalleSavedRegisters:
    # 1. this code can only be executed when we execute tasks (we do not return to callee from this code)
    # 2. even though we pushed R4 register we couldn't pop it as it is a callee saved register
    #    and we might be returning to suspended task which might rely on using previously stored R4 
    #    register value
    # 3. to return from exception restore PC value from the stack
    POP         {R12, PC}
    .size       SVC_Handler, .-SVC_Handler

################################################################################
# svc calls which do not immediately affect tasks execution order
################################################################################

labelScheduleTaskSvc:
    # call SVC C function
    BL          scheduleTaskSvc
    B           labelEndFunctionWithResult

labelScheduleSuspendedTaskSvc:
    # call SVC C function
    BL          scheduleSuspendedTaskSvc
    B           labelEndFunctionWithResult

labelEndFunctionWithResult:
    # store SVC OS call result on the stack (where R0 is stored) so it can be returned to calling function
    STR         R0, [R4]
    # restore original value of R4 register (which we modified) from the stack
    # to return from exception restore PC value from the stack
    POP         {R4, PC}

################################################################################
# critical error, loop forever
################################################################################

labelError:
    # repeat forever
    B           labelError

.end
