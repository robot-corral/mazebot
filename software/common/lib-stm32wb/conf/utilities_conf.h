#pragma once

#include "stm32/stm32wb55xx.h"

#include <string.h>

#define __WEAK __attribute__((weak))

#define UTILS_MEMSET8(dest, value, size) memset(dest, value, size);

#define UTILS_ENTER_CRITICAL_SECTION() uint32_t __primask_bit = __get_PRIMASK( );\
                                       __disable_irq( )

#define UTILS_EXIT_CRITICAL_SECTION() __set_PRIMASK(__primask_bit)
