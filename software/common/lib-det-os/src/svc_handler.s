.syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

    .globl  SVC_Handler
    .section .text.SVC_Handler
    .type   SVC_Handler, %function
SVC_Handler:
    # store return address and registers we are going to use on the stack
    PUSH    {R0-R2, LR}
    # as per spec ARM®v7-M Architecture Reference Manual
    # B1.5.8 Exception return behavior
    # 2nd bit (starting from zero) is:
    # - zero if return stack is main (MSP)
    # - one if return stack is process (PSP)
    TST     LR, #4
    ITE     NE
    MRSNE   R1, PSP
    MRSEQ   R1, MSP
    # load PC value so we can read previous instruction's SVC code
    # (previous instruction is SVS #imm where #imm is 8 bit value)
    LDR     R2, [R1, #40]
    # read previous instruction #imm argument value
    LDRB    R1, [R2, #-2]
    # call C function to process SVC OS call
    # R0 contains pointer to osCallParameters_t object
    BL      systemCallHandler
    # restore registers we used from the stack
    # to return from exception restore PC value from the stack
    POP     {R0-R2, PC}
    .size   SVC_Handler, .-SVC_Handler

.end
