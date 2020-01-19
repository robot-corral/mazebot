#include "system_clocks.h"
#include "global_data.h"
#include "error.h"

#include "utilities/otp.h"

static void initializeHse();
static void initializeRtc();
static void initializeClocks();

void initializeHse()
{
    OTP_ID0_t* p_otp = (OTP_ID0_t*) OTP_Read(0);
    if (p_otp)
    {
        LL_RCC_HSE_SetCapacitorTuning(p_otp->hse_tuning);
    }
}

void initializeClocks()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /*
     * Configure LSE Drive Capability 
     */
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /*
     * Configure the main internal regulator output voltage 
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /*
     * Initializes the CPU, AHB and APB busses clocks 
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | 
                                       RCC_OSCILLATORTYPE_HSE |
                                       RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        fatalError();
    }

    /*
    * Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers 
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4  |
                                  RCC_CLOCKTYPE_HCLK2  |
                                  RCC_CLOCKTYPE_HCLK   |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1  |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        fatalError();
    }

    /*
     * Initializes the peripherals clocks 
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS     |
                                               RCC_PERIPHCLK_RFWAKEUP |
                                               RCC_PERIPHCLK_RTC      |
                                               RCC_PERIPHCLK_USART1   |
                                               RCC_PERIPHCLK_LPUART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
    PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
    PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        fatalError();
    }

    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1 | LL_AHB1_GRP1_PERIPH_CRC | LL_AHB1_GRP1_PERIPH_DMAMUX1);
}

void initializeRtc()
{
    g_rtc.Instance = RTC;
    g_rtc.Init.HourFormat = RTC_HOURFORMAT_24;
    g_rtc.Init.AsynchPrediv = CFG_RTC_ASYNCH_PRESCALER;
    g_rtc.Init.SynchPrediv = CFG_RTC_SYNCH_PRESCALER;
    g_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    g_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    g_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&g_rtc) != HAL_OK)
    {
        fatalError();
    }

    /* Disable RTC registers write protection */
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_WAKEUP_SetClock(RTC, CFG_RTC_WUCKSEL_DIVIDER);
    /* Enable RTC registers write protection */
    LL_RTC_EnableWriteProtection(RTC);
}

void initializeSystemClock()
{
    initializeHse();
    initializeClocks();
    // initializeRtc();

    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
}
