#pragma once

#include <stdint.h>

uint16_t getSensorStatus();

uint16_t setSensorStatusFlags(uint16_t flags);

uint16_t resetSensorStatusFlags(uint16_t flags);
