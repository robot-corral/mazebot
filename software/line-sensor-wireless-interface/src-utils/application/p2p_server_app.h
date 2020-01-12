#pragma once

typedef enum
{
  P2PAOCE_CONN_HANDLE_EVT,
  P2PAOCE_DISCON_HANDLE_EVT,
} p2pAppOpcodeNotificationEvent_t;

typedef struct
{
  p2pAppOpcodeNotificationEvent_t p2pEvtAppOpcode;
  uint16_t                        connectionHandle;
} p2pAppConnectionHandleNotificationEvent_t;

void p2pAppInit();

void p2pAppNotification(p2pAppConnectionHandleNotificationEvent_t* pNotification);

void p2pLineSensorDataAvailable();
