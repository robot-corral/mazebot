#pragma once

#define raise_interrupt_priority() asm("CPSID I")
#define restore_interrupt_priority() asm("CPSIE I")
