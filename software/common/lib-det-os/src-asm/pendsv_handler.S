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

    .globl      PendSV_Handler
    .section    .text.PendSV_Handler
    .type       PendSV_Handler, %function
PendSV_Handler:
    # store original LR so we can return later
    # R12 is stored to keep stack 8 bytes align
    PUSH        {R12, LR}

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
    # call SVC C function
    BL          suspendCurrentAndMoveToNextTaskSvc

    # compare result store in R0 to R_PREVIOUS_TASK_CONTINUES_EXECUTION
    CMP         R0, R_PREVIOUS_TASK_CONTINUES_EXECUTION
    # R0 is less than R_PREVIOUS_TASK_CONTINUES_EXECUTION then go to labelError
    BLT         labelError
    # R0 equals R_PREVIOUS_TASK_CONTINUES_EXECUTION then go to labelPreviousTaskContinuesExecution
    BEQ         labelPreviousTaskContinuesExecution
    # comare result stored in R0 to R_RESUME_NEXT_TASK
    CMP         R0, R_RESUME_NEXT_TASK
    # R0 is less than R_RESUME_NEXT_TASK then go to labelStartNextTask
    BLO         labelStartNextTask
    # R0 equals R_RESUME_NEXT_TASK then go to labelResumeNextTask
    BEQ         labelResumeNextTask

    # remaining results require storing previous task registers first

    # load old task register storage address into R1
    POP         {R1}
    # load PSP address into R12
    MRS         R12, PSP
    # store all general registers into task register storage
    STM         R1, {R4, R5, R6, R7, R8, R9, R10, R11, R12}

    # compare result stored in R0 to R_SUSPEND_PREVIOUS_TASK_AND_START_NEXT_TASK
    CMP         R0, R_SUSPEND_PREVIOUS_TASK_AND_START_NEXT_TASK
    # R0 is equal to R_SUSPEND_PREVIOUS_TASK_AND_START_NEXT_TASK then go to labelSuspendPreviousTaskAndStartNextTask
    BEQ         labelSuspendPreviousTaskAndStartNextTask
    # R0 is greater than R_SUSPEND_PREVIOUS_TASK_AND_START_NEXT_TASK (equals to R_SUSPEND_PREVIOUS_TASK_AND_RESUME_NEXT_TASK) then go to labelSuspendPreviousTaskAndResumeNextTask
    BHI         labelSuspendPreviousTaskAndResumeNextTask

labelError:
    # loop forever
    B           labelError

labelStartNextTask:
    # load startNewTask parameters into R0, R1, R2
    POP         {R0, R1, R2}
    # skip last parameter as it's not used
    ADD         SP, SP, 4
    # call starkNewTask
    BL          starkNewTask
    # we are done go to the end of the function
    B           labelEndFunction

labelResumeNextTask:
    # load resumeTask parameter into R0
    POP         {R0}
    # skip last parameter as it's not used
    ADD         SP, SP, 12
    # load suspended task register values
    LDM         R0, {R4, R5, R6, R7, R8, R9, R10, R11, R12}
    # save stack address from R12 to PSP
    MSR         PSP, R12
    # we are done go to the end of the function
    B           labelEndFunction

labelSuspendPreviousTaskAndStartNextTask:
    # load startNewTask parameters into R0, R1, R2
    POP         {R0, R1, R2}
    # call starkNewTask
    BL          starkNewTask
    # we are done go to the end of the function
    B           labelEndFunction

labelSuspendPreviousTaskAndResumeNextTask:
    # load resumeTask parameters into R0, R1
    POP         {R0}
    # skip last parameter as it's not used
    ADD         SP, SP, 8
    # load suspended task register values
    LDM         R0, {R4, R5, R6, R7, R8, R9, R10, R11, R12}
    # save stack address from R12 to PSP
    MSR         PSP, R12
    # we are done go to the end of the function
    B           labelEndFunction

labelPreviousTaskContinuesExecution:
    # free 4 * 4 bytes from the stack (discard values)
    ADD         SP, SP, 16

labelEndFunction:
    # return to calling function
    POP         {R12, PC}
    .size       PendSV_Handler, .-PendSV_Handler

.end
