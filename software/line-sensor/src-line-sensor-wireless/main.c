#include "system.h"

#include <sequencer/stm32_seq.h>

int main()
{
    initializeSystem();

    while (1)
    {
        UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
    }
}

void UTIL_SEQ_Idle()
{
}
