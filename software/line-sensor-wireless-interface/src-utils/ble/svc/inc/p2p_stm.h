#pragma once

#include <line_sensor.h>

#define P2P_CLIENT_COMMAND_LENGTH  2
#define P2P_CLIENT_RESPONSE_LENGTH ((NUMBER_OF_SENSORS) * sizeof(lineSensorValue_t))

typedef enum
{
    P2PS_NONE,
    P2PS_CLIENT_COMMAND,
    P2PS_CLIENT_ENABLE_RESPONSE,
    P2PS_CLIENT_DISABLE_RESPONSE
} p2pOpcode_t;

typedef struct
{
    uint8_t* pPayload;
    uint8_t  length;
} p2pData_t;

typedef struct
{
    p2pOpcode_t p2pOpcode;
    p2pData_t   dataTransfered;
    uint16_t    connectionHandle;
    uint8_t     serviceInstance;
} p2pStmAppNotification_t;

/*
 * declared as weak function in svc_ctl.c
 */
void P2PS_STM_Init();

void p2pStmAppNotification(p2pStmAppNotification_t* pNotification);

/*
 * uuid - one of {P2P_CLIENT_COMMAND_UUID, P2P_CLIENT_RESPONSE_UUID}
 */
tBleStatus p2pAppUpdateChar(uint16_t uuid, uint8_t* pPayload);
