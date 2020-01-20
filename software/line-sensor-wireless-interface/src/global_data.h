#pragma once

#include <line_sensor.h>
#include <consumer_producer_buffer.h>

#include <stm32/stm32wb55xx.h>

#include "stm32wbxx_hal.h"
#include "ble_thread/tl/tl.h"
#include "application/app_common.h"

extern RTC_HandleTypeDef g_rtc;
extern UART_HandleTypeDef g_uart1;

#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH*4U*DIVC(( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ), 4U))

PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t g_evtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t g_systemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t g_systemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t g_bleSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];

/*******************************************************************************
 * Task scheduler
 *******************************************************************************/

extern volatile bool g_lcsRestart;
extern volatile bool g_lscIsActive;
extern volatile bool g_lscIsReceiving;
extern volatile bool g_lscIsTransmitting;
extern volatile lineSensorRequest_t g_lscTxBuffer;
extern volatile lineSensorResponse_t g_lscRxBuffer;
extern volatile lineSensorCommandCode_t g_lscCurrentCommandCode;

/*******************************************************************************
 * Data queue
 *******************************************************************************/

typedef struct __attribute__((packed))
{
    lineSensorValue_t sensorValues[LINE_SENSOR_NUMBER_OF_SENSORS];
    uint8_t numberOfCalls;
    uint8_t numberOfFailures;
    uint8_t numberOfCrcErrors;
    lineSensorStatus_t currentStatus;
    lineSensorDetailedStatus_t currentDetailedStatus;
    lineSensorDetailedStatus_t cumulativeDetailedStatusSinceLastReset;
} lineSensorData_t;

#define NUMBER_OF_LINE_SENSOR_DATA_BUFFERS DATA_BUFFER_LENGTH

extern volatile uint32_t g_lineSensorDataQueueProducerConsumerIndexes;
extern volatile lineSensorData_t g_lineSensorDataQueue[NUMBER_OF_LINE_SENSOR_DATA_BUFFERS];
