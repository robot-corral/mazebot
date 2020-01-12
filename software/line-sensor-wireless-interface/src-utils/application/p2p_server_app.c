#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "p2p_server_app.h"
#include "stm32_seq.h"
#include "line_sensor.h"

typedef struct
{
    uint8_t           notificationStatus;
    lineSensorValue_t lineSensorValues[NUMBER_OF_SENSORS];
    uint16_t          connectionHandle;
} p2pServerAppContext_t;

PLACE_IN_SECTION("BLE_APP_CONTEXT") static p2pServerAppContext_t g_p2pServerAppContext;

static void p2pSendNotification(void);

void p2pStmAppNotification(p2pStmAppNotification_t* pNotification)
{
    switch(pNotification->p2pOpcode)
    {
        case P2PS_CLIENT_ENABLE_RESPONSE:
        {
            g_p2pServerAppContext.notificationStatus = 1;
            break;
        }
        case P2PS_CLIENT_DISABLE_RESPONSE:
        {
            g_p2pServerAppContext.notificationStatus = 0;
            break;
        }
        default:
        {
            break;
        }
    }
}

void p2pAppNotification(p2pAppConnectionHandleNotificationEvent_t* pNotification)
{
    switch(pNotification->p2pEvtAppOpcode)
    {
        case P2PAOCE_CONN_HANDLE_EVT:
            break;
        case P2PAOCE_DISCON_HANDLE_EVT :
            break;
        default:
            break;
    }
}

void p2pAppInit()
{
    UTIL_SEQ_RegTask(1 << CFG_TASK_LINE_SENSOR_DATA_AVAILABLE, UTIL_SEQ_RFU, p2pSendNotification);
    g_p2pServerAppContext.notificationStatus = 0; 
}

void p2pLineSensorDataAvailable()
{
    UTIL_SEQ_SetTask(1 << CFG_TASK_LINE_SENSOR_DATA_AVAILABLE, CFG_SCH_PRIO_0);
}

void p2pSendNotification()
{
    if(g_p2pServerAppContext.notificationStatus)
    {
        p2pAppUpdateChar(P2P_CLIENT_RESPONSE_UUID, (uint8_t*) &g_p2pServerAppContext.lineSensorValues);
    }
}
