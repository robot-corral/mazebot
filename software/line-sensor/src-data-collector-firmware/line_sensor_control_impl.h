#pragma once

#include <stdint.h>

void checkIfSpiCommunicationHasFinished();

void createGetSensorValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength);
void createStartCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength, uint8_t parameter);
void createGetCalibrationValuesCommand(uint16_t* pRxLength, uint16_t* pTxLength);
void createFinishCalibrationCommand(uint16_t* pRxLength, uint16_t* pTxLength);
void createGetDetailedSensorStatusCommand(uint16_t* pRxLength, uint16_t* pTxLength);
void createResetCommand(uint16_t* pRxLength, uint16_t* pTxLength);
