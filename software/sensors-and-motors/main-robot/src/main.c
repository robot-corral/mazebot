/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "driver.h"
#include "motor.h"

int main()
{
    initializeDriver();
    initializeDebugDriver(true);

    for (;;) ;
}
