#include "..\src\output.h"

#include <stm32/stm32l4xx_ll_bus.h>
#include <stm32/stm32l4xx_ll_pwr.h>
#include <stm32/stm32l4xx_ll_rcc.h>
#include <stm32/stm32l4xx_ll_tim.h>
#include <stm32/stm32l4xx_ll_gpio.h>
#include <stm32/stm32l4xx_ll_utils.h>
#include <stm32/stm32l4xx_ll_lpuart.h>
#include <stm32/stm32l4xx_ll_system.h>

static void initializeSystemClock();
static void initializeClockTimer();
static void enableVddIo2();
static void initializeGpio();
static void initializeUart();

void initializeHardware()
{
    initializeSystemClock();
    initializeClockTimer();
    initializeGpio();
    initializeUart();
}

void initializeSystemClock()
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    LL_RCC_HSI48_Enable();
    while (LL_RCC_HSI48_IsReady() != 1);

    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() != 1);

    LL_RCC_MSI_Enable();
    while (LL_RCC_MSI_IsReady() != 1);

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
    LL_RCC_MSI_SetCalibTrimming(0);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();

    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_Init1msTick(80000000);
    LL_SetSystemCoreClock(80000000);

    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5 |
                             LL_APB1_GRP1_PERIPH_PWR);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOG |
                             LL_APB2_GRP1_PERIPH_SYSCFG);
}

void initializeClockTimer()
{
    LL_TIM_SetPrescaler(TIM5, __LL_TIM_CALC_PSC(SystemCoreClock, 1000000));
    LL_TIM_SetAutoReload(TIM5, 0xFFFFFFFF); // reload 32 bit value (TIM5 is 32 bit)
    LL_TIM_EnableCounter(TIM5);
    LL_TIM_GenerateEvent_UPDATE(TIM5);
}

void enableVddIo2()
{
    if (LL_PWR_IsEnabledVddIO2())
    {
        return;
    }

    LL_PWR_EnablePVM(LL_PWR_PVM_VDDIO2_0_9V);

    while (LL_PWR_IsActiveFlag_PVMO2());

    do
    {
        LL_PWR_EnableVddIO2();
    } while (!LL_PWR_IsEnabledVddIO2());

    LL_PWR_DisablePVM(LL_PWR_PVM_VDDIO2_0_9V);
}

void initializeGpio()
{
    enableVddIo2();

    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOG, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOG, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);
    LL_GPIO_SetAFPin_0_7(GPIOG, LL_GPIO_PIN_7, LL_GPIO_AF_8);

    LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOG, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOG, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOG, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);
    LL_GPIO_SetAFPin_8_15(GPIOG, LL_GPIO_PIN_8, LL_GPIO_AF_8);
}

void initializeUart()
{
    LL_LPUART_SetTransferDirection(LPUART1, LL_LPUART_DIRECTION_TX);
    LL_LPUART_ConfigCharacter(LPUART1, LL_LPUART_DATAWIDTH_8B, LL_LPUART_PARITY_NONE, LL_LPUART_STOPBITS_1);
    LL_LPUART_SetBaudRate(LPUART1, SystemCoreClock, 230400);

    LL_LPUART_Enable(LPUART1);

    while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1));
}

void __attribute__((weak)) printString(const char* pMessage, size_t messageLength)
{
    if (messageLength == 0)
    {
        return;
    }

    const uint32_t lengthMinusOne = messageLength - 1;

    for (uint32_t i = 0; i < lengthMinusOne; ++i)
    {
        while (!LL_LPUART_IsActiveFlag_TXE(LPUART1));
        LL_LPUART_TransmitData8(LPUART1, pMessage[i]);
    }

    while (!LL_LPUART_IsActiveFlag_TXE(LPUART1));
    LL_LPUART_ClearFlag_TC(LPUART1);
    LL_LPUART_TransmitData8(LPUART1, pMessage[messageLength - 1]);

    while (!LL_LPUART_IsActiveFlag_TC(LPUART1));
}
