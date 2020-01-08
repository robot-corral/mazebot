#pragma once

#include <stdbool.h>

void setEmergencyStopLedEnabled(bool isEnabled);

void setFatalErrorLedEnabled(bool isEnabled);

void setPositionControllerMovingLedEnabled(bool isEnabled);

void setTxErrorLedEnabled(bool isEnabled);
void setTxActiveLedEnabled(bool isEnabled);

void setRxErrorLedEnabled(bool isEnabled);
void setRxActiveLedEnabled(bool isEnabled);
