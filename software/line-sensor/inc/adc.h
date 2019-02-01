/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#pragma once

/*
 * initializes ADC hardware.
 *
 * should only be called once after reset.
 */
void initializeAdc();

/*
 * starts the process of querying ADC data. This call is asynchronous.
 */
void startQueryingAdc();
