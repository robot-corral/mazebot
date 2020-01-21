#pragma once

#include <stdbool.h>
#include <line_sensor.h>

void initializeLineSensor();

void startQueryingLineSensor();
void stopQueryingLineSensor();

void lineSensorDataAvailable();
