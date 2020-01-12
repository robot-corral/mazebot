#include "common_blesvc.h"

typedef struct{
    uint16_t p2pSvcHdle;
    uint16_t p2pClientCommandCharHdle;
    uint16_t p2pClientResponseCharHdle;
} p2pContext_t;

PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static p2pContext_t g_p2pContext;

static SVCCTL_EvtAckStatus_t p2pEventHandler(void* pEvent);

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[ 0] = uuid_0;  uuid_struct[ 1] = uuid_1;  uuid_struct[ 2] = uuid_2;  uuid_struct[ 3] = uuid_3;  \
    uuid_struct[ 4] = uuid_4;  uuid_struct[ 5] = uuid_5;  uuid_struct[ 6] = uuid_6;  uuid_struct[ 7] = uuid_7;  \
    uuid_struct[ 8] = uuid_8;  uuid_struct[ 9] = uuid_9;  uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
    uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_P2P_SERVICE_UUID(uuid_struct)         COPY_UUID_128(uuid_struct, 0x00, 0x00, 0xfe, 0x40, 0xcc, 0x7a, 0x48, 0x2a, 0x98, 0x4a, 0x7f, 0x2e, 0xd5, 0xb3, 0xe5, 0x8f)
#define COPY_P2P_CLIENT_COMMAND_UUID(uuid_struct)  COPY_UUID_128(uuid_struct, 0x00, 0x00, 0xfe, 0x41, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19)
#define COPY_P2P_CLIENT_RESPONSE_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x00, 0x00, 0xfe, 0x42, 0x8e, 0x22, 0x45, 0x41, 0x9d, 0x4c, 0x21, 0xed, 0xae, 0x82, 0xed, 0x19)

static SVCCTL_EvtAckStatus_t p2pEventHandler(void* pEvent)
{
    p2pStmAppNotification_t pNotification;

    SVCCTL_EvtAckStatus_t returnValue = SVCCTL_EvtNotAck;
    hci_event_pckt* pEventPacket = (hci_event_pckt *) (((hci_uart_pckt*) pEvent)->data);

    if (pEventPacket->evt == EVT_VENDOR)
    {
        evt_blue_aci* pBlueEvent = (evt_blue_aci*) pEventPacket->data;

        if (pBlueEvent->ecode == EVT_BLUE_GATT_ATTRIBUTE_MODIFIED)
        {
            aci_gatt_attribute_modified_event_rp0* pAttributeModified = (aci_gatt_attribute_modified_event_rp0*) pBlueEvent->data;

            if(pAttributeModified->Attr_Handle == (g_p2pContext.p2pClientCommandCharHdle + 1))
            {
                pNotification.p2pOpcode               = P2PS_CLIENT_COMMAND;
                pNotification.dataTransfered.length   = pAttributeModified->Attr_Data_Length;
                pNotification.dataTransfered.pPayload = pAttributeModified->Attr_Data;
                p2pStmAppNotification(&pNotification);
            }
            else if(pAttributeModified->Attr_Handle == (g_p2pContext.p2pClientResponseCharHdle + 2))
            {
                returnValue = SVCCTL_EvtAckFlowEnable;
                if(pAttributeModified->Attr_Data[0] & COMSVC_Notification)
                {
                    pNotification.p2pOpcode = P2PS_CLIENT_ENABLE_RESPONSE;
                    p2pStmAppNotification(&pNotification);
                }
                else
                {
                    pNotification.p2pOpcode = P2PS_CLIENT_DISABLE_RESPONSE;
                    p2pStmAppNotification(&pNotification);
                }
            }
        }
    }

    return returnValue;
}

void P2PS_STM_Init(void)
{
    SVCCTL_RegisterSvcHandler(p2pEventHandler);

    Char_UUID_t uuid;
    tBleStatus status;

    COPY_P2P_SERVICE_UUID(uuid.Char_UUID_128);
    status = aci_gatt_add_service(UUID_TYPE_128,
                                  (Service_UUID_t*) &uuid,
                                  PRIMARY_SERVICE,
                                  8,
                                  &g_p2pContext.p2pSvcHdle);

    COPY_P2P_CLIENT_COMMAND_UUID(uuid.Char_UUID_128);
    status = aci_gatt_add_char(g_p2pContext.p2pSvcHdle,
                               UUID_TYPE_128,
                               &uuid,
                               P2P_CLIENT_COMMAND_LENGTH,
                               CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_READ,
                               ATTR_PERMISSION_NONE,
                               GATT_NOTIFY_ATTRIBUTE_WRITE,
                               10,
                               1,
                               &g_p2pContext.p2pClientCommandCharHdle);

    COPY_P2P_CLIENT_RESPONSE_UUID(uuid.Char_UUID_128);
    status = aci_gatt_add_char(g_p2pContext.p2pSvcHdle,
                               UUID_TYPE_128,
                               &uuid,
                               P2P_CLIENT_RESPONSE_LENGTH,
                               CHAR_PROP_NOTIFY,
                               ATTR_PERMISSION_NONE,
                               GATT_NOTIFY_ATTRIBUTE_WRITE,
                               10,
                               1,
                               &g_p2pContext.p2pClientResponseCharHdle);
}

tBleStatus p2pAppUpdateChar(uint16_t uuid, uint8_t* pPayload)
{
    tBleStatus result = BLE_STATUS_INVALID_PARAMS;

    switch(uuid)
    {
        case P2P_CLIENT_COMMAND_UUID:
        {
            result = aci_gatt_update_char_value(g_p2pContext.p2pSvcHdle,
                                                g_p2pContext.p2pClientCommandCharHdle,
                                                0,
                                                P2P_CLIENT_COMMAND_LENGTH,
                                                (uint8_t*) pPayload);
            break;
        }
        case P2P_CLIENT_RESPONSE_UUID:
        {
            result = aci_gatt_update_char_value(g_p2pContext.p2pSvcHdle,
                                                g_p2pContext.p2pClientResponseCharHdle,
                                                0,
                                                P2P_CLIENT_RESPONSE_LENGTH,
                                                (uint8_t*) pPayload);
            break;
        }
    }

    return result;
}
