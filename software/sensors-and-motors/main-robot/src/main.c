/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "driver.h"

int main()
{
    initializeDriver();
    initializeDebugDriver(true);

    for (;;) ;
}
