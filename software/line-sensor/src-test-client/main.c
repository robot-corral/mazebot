#include "line_sensor.h"

#include <stm32\stm32l4xx_ll_bus.h>
#include <stm32\stm32l4xx_ll_dma.h>
#include <stm32\stm32l4xx_ll_pwr.h>
#include <stm32\stm32l4xx_ll_rcc.h>
#include <stm32\stm32l4xx_ll_tim.h>
#include <stm32\stm32l4xx_ll_exti.h>
#include <stm32\stm32l4xx_ll_gpio.h>
#include <stm32\stm32l4xx_ll_utils.h>
#include <stm32\stm32l4xx_ll_usart.h>
#include <stm32\stm32l4xx_ll_cortex.h>
#include <stm32\stm32l4xx_ll_system.h>

lineSensorCommand_t g_txBuffer;

#define RX_BUFFER_LENGTH (sizeof(lineSensorCommandResponse_t) + 10)

uint8_t g_rxBuffer[RX_BUFFER_LENGTH];

#define LL_USART3_DMA_CHANNEL_TX LL_DMA_CHANNEL_2
#define LL_USART3_DMA_CHANNEL_RX LL_DMA_CHANNEL_3

static void sendCommand();

void SystemClock_Config()
{
    LL_AHB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
    LL_RCC_MSI_SetCalibTrimming(0);
    LL_RCC_MSI_Enable();

    while (LL_RCC_MSI_IsReady() != 1) ;

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();

    while (LL_RCC_PLL_IsReady() != 1) ;

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) ;

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(80000000);
    LL_SetSystemCoreClock(80000000);

    LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

    LL_TIM_SetPrescaler(TIM5, __LL_TIM_CALC_PSC(SystemCoreClock, 1000000));
    LL_TIM_SetAutoReload(TIM5, 0xFFFFFFFF); // reload 32 bit value (TIM5 is 32 bit)
    LL_TIM_EnableCounter(TIM5);
    LL_TIM_GenerateEvent_UPDATE(TIM5);
}

void initializeButton()
{
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_NO);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);

    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
}

int main()
{
    SystemClock_Config();
    initializeButton();

    for (;;) ;
}

void DMA1_Channel2_IRQHandler(void)
{
    // TX

    if (LL_DMA_IsActiveFlag_TC2(DMA1))
    {
        LL_DMA_ClearFlag_TC2(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE2(DMA1))
    {
        for (;;);
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    // RX

    if (LL_DMA_IsActiveFlag_TC3(DMA1))
    {
        LL_DMA_ClearFlag_TC3(DMA1);
    }
    else if (LL_DMA_IsActiveFlag_TE3(DMA1))
    {
        for (;;) ;
    }
}

void USART3_IRQHandler()
{
    if (LL_USART_IsActiveFlag_IDLE(USART3))
    {
        // TODO
    }
    if (LL_USART_IsActiveFlag_PE(USART3))
    {
        // TODO
    }
    if (LL_USART_IsActiveFlag_RTO(USART3))
    {
        // TODO
    }
    if (LL_USART_IsActiveFlag_FE(USART3))
    {
        // TODO
    }
    if (LL_USART_IsActiveFlag_NE(USART3))
    {
        // TODO
    }
    if (LL_USART_IsActiveFlag_ORE(USART3))
    {
        // TODO
    }
}

void EXTI15_10_IRQHandler()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        sendCommand();
    }
}

void sendCommand()
{
    g_txBuffer.header.prefix = COMMAND_PREFIX;
    g_txBuffer.header.commandCode = LSC_SEND_SENSOR_DATA;

    LL_DMA_DisableChannel(DMA1, LL_USART3_DMA_CHANNEL_RX);
    LL_DMA_DisableChannel(DMA1, LL_USART3_DMA_CHANNEL_TX);

    LL_DMA_SetDataLength(DMA1, LL_USART3_DMA_CHANNEL_RX, RX_BUFFER_LENGTH);
    LL_DMA_SetDataLength(DMA1, LL_USART3_DMA_CHANNEL_TX, LSC_LENGTH_SIMPLE_COMMAND);

    LL_DMA_EnableChannel(DMA1, LL_USART3_DMA_CHANNEL_RX);
    LL_DMA_EnableChannel(DMA1, LL_USART3_DMA_CHANNEL_TX);
}
