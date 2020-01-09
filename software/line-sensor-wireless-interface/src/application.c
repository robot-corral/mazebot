#include "application.h"
#include "global_data.h"

#include "sequencer/stm32_seq.h"

#include "tiny_lpm/stm32_lpm.h"

#include "application/hw_if.h"
#include "application/app_ble.h"
#include "application/app_conf.h"

#include "ble_thread/tl/tl.h"
#include "ble_thread/tl/shci_tl.h"

static void initializeDebug();

static void sysStatusNot(SHCI_TL_CmdStatus_t status);
static void sysUserEvtRx(void * pPayload);

void initializeApplication()
{
    UTIL_LPM_Init();

    HW_TS_Init(hw_ts_InitMode_Full, &g_rtc);

    UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

    /* Reference table initialization */
    TL_Init();

    TL_MM_Config_t tl_mm_config;
    SHCI_TL_HciInitConf_t shci_tl_init_conf;

    /**< System channel initialization */
    UTIL_SEQ_RegTask(1 << CFG_TASK_SYSTEM_HCI_ASYNCH_EVT_ID, UTIL_SEQ_RFU, shci_user_evt_proc);
    shci_tl_init_conf.p_cmdbuffer = (uint8_t*) &g_systemCmdBuffer;
    shci_tl_init_conf.StatusNotCallBack = sysStatusNot;
    shci_init(sysUserEvtRx, (void*) &shci_tl_init_conf);

    /* Memory Manager channel initialization */
    tl_mm_config.p_BleSpareEvtBuffer = g_bleSpareEvtBuffer;
    tl_mm_config.p_SystemSpareEvtBuffer = g_systemSpareEvtBuffer;
    tl_mm_config.p_AsynchEvtPool = g_evtPool;
    tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
    TL_MM_Init( &tl_mm_config );

    TL_Enable();
}

void EXTI4_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(BUTTON_SW1_PIN);
}

void EXTI0_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(BUTTON_SW2_PIN);
}

void RTC_WKUP_IRQHandler(void)
{
    HW_TS_RTC_Wakeup_Handler();
}

void IPCC_C1_TX_IRQHandler(void)
{
    HW_IPCC_Tx_Handler();
}

void IPCC_C1_RX_IRQHandler(void)
{
    HW_IPCC_Rx_Handler();
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
    switch (GPIO_Pin)
    {
        case BUTTON_SW1_PIN:
        {
            APP_BLE_Key_Button1_Action();
            break; 
        }
        case BUTTON_SW2_PIN:
        {
            APP_BLE_Key_Button2_Action();
            break; 
        }
        case BUTTON_SW3_PIN:
        {
            APP_BLE_Key_Button3_Action();
            break;
        }
        default: break;
    }
}

void DbgOutputTraces(uint8_t *p_data, uint16_t size, void (*cb) (void))
{
}

void sysStatusNot(SHCI_TL_CmdStatus_t status)
{
}

void sysUserEvtRx(void * pPayload)
{
    TL_TRACES_Init();
    APP_BLE_Init();
    UTIL_LPM_SetOffMode(1U << CFG_LPM_APP, UTIL_LPM_ENABLE);
}

void shci_notify_asynch_evt(void* pdata)
{
    UTIL_SEQ_SetTask(1 << CFG_TASK_SYSTEM_HCI_ASYNCH_EVT_ID, CFG_SCH_PRIO_0);
}

void shci_cmd_resp_release(uint32_t flag)
{
    UTIL_SEQ_SetEvt(1 << CFG_IDLEEVT_SYSTEM_HCI_CMD_EVT_RSP_ID);
}

void shci_cmd_resp_wait(uint32_t timeout)
{
    UTIL_SEQ_WaitEvt(1 << CFG_IDLEEVT_SYSTEM_HCI_CMD_EVT_RSP_ID);
}
