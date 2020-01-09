#pragma once

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
