/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

/*
 * this file has capital .S extension to enable preprocessor expnding definitions
 */

#include "../src/schedule_tasks_defs.h"

.syntax unified
    .cpu        cortex-m4
    .fpu        vfpv2
    .thumb

    .globl      PendSV_Handler_ProcessTaskSchedulerResult
    .section    .text.PendSV_Handler_ProcessTaskSchedulerResult
    .type       PendSV_Handler_ProcessTaskSchedulerResult, %function

    .globl      PendSV_Handler
    .section    .text.PendSV_Handler
    .type       PendSV_Handler, %function

PendSV_Handler:
    # allocate memory for return parameters from moveToNextTaskSvc function
    SUB         SP, SP, 16
    # save address of 1st output parameter to R0
    MOV         R0, SP
    # save address of 2nd output parameter to R1
    ADD         R1, SP, 4
    # save address of 3rd output parameter to R2
    ADD         R2, SP, 8
    # save address of 3rd output parameter to R3
    ADD         R3, SP, 12
    # save return address onto a stack (calling suspendCurrentAndMoveToNextTaskSvc will change it)
    PUSH        {LR}
    # call SVC C function
    BL          suspendCurrentAndMoveToNextTaskSvc
    # restore LR from the stack
    POP         {R14}

PendSV_Handler_ProcessTaskSchedulerResult:
    # compare result store in R0 to R_PREVIOUS_TASK_CONTINUES_EXECUTION
    CMP         R0, R_PREVIOUS_TASK_CONTINUES_EXECUTION
    # R0 is less than R_PREVIOUS_TASK_CONTINUES_EXECUTION then go to labelError
    BLT         labelError
     # R0 equals R_PREVIOUS_TASK_CONTINUES_EXECUTION then go to labelPreviousTaskContinuesExecution
    BEQ         labelPreviousTaskContinuesExecution
    # compare result stored in R0 to R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK
    CMP         R0, R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK
    # result <= R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK then go to labelSuspendPreviousTaskFirst
    BLE         labelSuspendPreviousTaskFirst

    ############################################################################
    # no old task so we can simply start or resume next task
    ############################################################################

    # compare result stored in R0 to R_RESUME_NEXT_TASK
    CMP         R0, R_RESUME_NEXT_TASK
    # restore start / resume next task parameters
    POP         {R0, R1, R2}
    # skip last parameter as it's not used
    ADD         SP, SP, 4

    # result == R_RESUME_NEXT_TASK then go to labelResumeNextTask
    BEQ         labelResumeNextTask
    # result < R_RESUME_NEXT_TASK (equivalently == R_START_NEXT_TASK) then go to labelResumeNextTask
    BLT         labelStartNextTask
    # otherwise it is an error
    B           labelError

    ############################################################################
    # remaining results require saving previous task context first
    ############################################################################

labelSuspendPreviousTaskFirst:
    # load old task register storage address into R1
    POP         {R1}
    # load PSP address into R12
    MRS         R12, PSP
    # store all general registers into task register storage
    STM         R1, {R4, R5, R6, R7, R8, R9, R10, R11, R12}
    # compare result stored in R0 to R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK
    CMP         R0, R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK
    # restore start next task / resume next task arguments
    POP         {R0, R1, R2}

    # result == R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK then go to labelResumeNextTask
    BEQ         labelResumeNextTask
    # result < R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK (equivalently == R_SUSPEND_PREVIOUS_TASK_AND_START_NEXT_TASK) then go to labelStartNextTask
    BLT         labelStartNextTask

labelError:
    # loop forever
    B           labelError

labelStartNextTask:
    # occupy 8 words (4-byte) on the task specific stack
    SUB         R2, 32
    # store next task stack address from R2 to PSP
    MSR         PSP, R2
    # save task parameter from R0 on next task stack (at location where R0 is stored by exception handler)
    STR         R0, [R2]
    # load reset value of xPSR register to R3
    # (for now we only set thumb instructions bit, might need to make this code more versatile)
    MOV         R3, 0b00000001000000000000000000000000
    # load address of SVC 0 to R0
    LDR         R0, =executeTasksOnPsp
    # make R2 point to LR on the new process stack
    ADD         R2, R2, 20
    # save 3 registers in one instruction
    # save previous ReturnAddress from R0 into LR on the next task stack
    # save task start address from R1 into ReturnAddress on the next task stack
    # save previous xPSR from R3 into xPSR on the next task stack
    STM         R2, {R0, R1, R3}
    # return to calling function
    BX          LR

labelResumeNextTask:
    # load suspended task register values
    LDM         R0, {R4, R5, R6, R7, R8, R9, R10, R11, R12}
    # save stack address from R12 to PSP
    MSR         PSP, R12
    # return to calling function
    BX          LR

labelPreviousTaskContinuesExecution:
    # free 4 * 4 bytes from the stack (discard values)
    ADD         SP, SP, 16
    # return to calling function
    BX          LR

    .size       PendSV_Handler, .-PendSV_Handler

.end
