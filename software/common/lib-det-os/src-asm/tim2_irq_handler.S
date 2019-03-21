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

    .globl      TIM2_IRQHandler
    .section    .text.TIM2_IRQHandler
    .type       TIM2_IRQHandler, %function
TIM2_IRQHandler:
    # store original LR so we can return later
    # R12 is stored to keep stack 8 bytes align
    PUSH        {R12, LR}

    # store TIM2 base address to R0
    MOV         R0, 0x40000000
    # store ~0x1 into R1
    MVN         R1, 0x1
    # clear bit 1 (flag UIF) in TIM2_SR register
    STR         R1, [R0, 0x10]

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

    # compare result store in R0 to R_OLD_TASK_CONTINUES_EXECUTION == 1
    CMP         R0, R_OLD_TASK_CONTINUES_EXECUTION
    # R0 is less than R_OLD_TASK_CONTINUES_EXECUTION then go to labelError
    BLT         labelError
    # R0 equals R_OLD_TASK_CONTINUES_EXECUTION then go to labelOldTaskContinuesExecution
    BEQ         labelOldTaskContinuesExecution
    # comare result stored in R0 to R_RESUME_NEW_TASK == 3
    CMP         R0, R_RESUME_TASK
    # R0 is less than R_RESUME_NEW_TASK then go to labelStartNewTask
    BLO         labelStartNewTask
    # R0 equals R_RESUME_NEW_TASK then go to labelResumeNewTask
    BEQ         labelResumeNewTask

    # remaining results require storing previous task registers first

    # load old task register storage address into R1
    POP         {R1}
    # load PSP address into R12
    MRS         R12, PSP
    # store all general registers into task register storage
    STM         R1, {R4, R5, R6, R7, R8, R9, R10, R11, R12}

    # compare result stored in R0 to R_SUSPEND_OLD_TASK_START_NEW_TASK == 4
    CMP         R0, R_SUSPEND_OLD_TASK_START_NEW_TASK
    # R0 is equal to R_SUSPEND_OLD_TASK_START_NEW_TASK then go to labelSuspendOldTaskStartNewTask
    BEQ         labelSuspendOldTaskStartNewTask
    # R0 is greater than R_SUSPEND_OLD_TASK_START_NEW_TASK (equals to R_SUSPEND_OLD_TASK_RESUME_NEW_TASK == 5) then go to labelSuspendOldTaskResumeNewTask
    BHI         labelSuspendOldTaskResumeNewTask

labelError:
    # loop forever
    B           labelError

labelStartNewTask:
    # load startNewTask parameters into R0, R1, R2
    POP         {R0, R1, R2}
    # skip last parameter as it's not used
    ADD         SP, SP, 4
    # call starkNewTask
    BL          starkNewTask
    # we are done go to the end of the function
    B           labelEndFunction

labelResumeNewTask:
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

labelSuspendOldTaskStartNewTask:
    # load startNewTask parameters into R0, R1, R2
    POP         {R0, R1, R2}
    # call starkNewTask
    BL          starkNewTask
    # we are done go to the end of the function
    B           labelEndFunction

labelSuspendOldTaskResumeNewTask:
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

labelOldTaskContinuesExecution:
    # free 4 * 4 bytes from the stack (discard values)
    ADD         SP, SP, 16

labelEndFunction:
    # return to calling function
    POP         {R12, PC}
    .size       TIM2_IRQHandler, .-TIM2_IRQHandler

.end
