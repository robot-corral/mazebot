/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "tasks.h"
#include "driver.h"

int main()
{
    initializeDriver();
    initializeDebugDriver(true);
    startTaskScheduler();

    for (;;) ;
}
