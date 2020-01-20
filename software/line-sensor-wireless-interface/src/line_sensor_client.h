#pragma once

#include <stdbool.h>
#include <line_sensor.h>

void initializeLineSensor();

void startQueryingLineSensor();
void stopQueryingLineSensor();

// source is in src-utils/application/p2p_server_app.c
// whole thing will be refactored later on
void lineSensorDataAvailable();
