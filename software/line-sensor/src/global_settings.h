/*******************************************************************************
 * Copyright (C) 2019 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

// #define CRYSTAL_EXTERNAL_HSE
// #define CRYSTAL_EXTERNAL_LSE

#ifdef CRYSTAL_EXTERNAL_HSE
    #define HSE_VALUE 24000000U  /*!< Value of the HSE oscillator in Hz */
#endif