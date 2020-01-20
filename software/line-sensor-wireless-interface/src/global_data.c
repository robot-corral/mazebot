#include "global_data.h"

RTC_HandleTypeDef g_rtc;
UART_HandleTypeDef g_uart1;

/*******************************************************************************
 * Task scheduler
 *******************************************************************************/

volatile bool g_lcsRestart;
volatile bool g_lscIsActive;
volatile bool g_lscIsReceiving;
volatile bool g_lscIsTransmitting;
volatile lineSensorRequest_t g_lscTxBuffer;
volatile lineSensorResponse_t g_lscRxBuffer;
volatile lineSensorCommandCode_t g_lscCurrentCommandCode;

/*******************************************************************************
 * Data queue
 *******************************************************************************/

volatile uint32_t g_lineSensorDataQueueProducerConsumerIndexes;
volatile lineSensorData_t g_lineSensorDataQueue[NUMBER_OF_LINE_SENSOR_DATA_BUFFERS];
