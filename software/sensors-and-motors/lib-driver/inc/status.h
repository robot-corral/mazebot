/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

typedef enum
{
    ERROR_MOTOR = 0x01,
} Error_t;

void setCriticalError(Error_t error);
