#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"
#include "usart.h"
#include "system_clocks.h"

#include <stm32/stm32l1xx_ll_pwr.h>
#include <stm32/stm32l1xx_ll_cortex.h>



#include <stm32/stm32l1xx_ll_gpio.h>
#include <stm32/stm32l1xx_ll_utils.h>

void main()
{
    communicationInterface_t communicationInterface = getCommunicationInterface();
    initializeSystemClocks(communicationInterface);
    initializeGpio(communicationInterface);
    initializeAdc();
    switch (communicationInterface)
    {
        case CI_I2C: initializeI2c(); break;
        case CI_SPI: initializeSpi(); break;
        case CI_USART: initializeUsart(); break;
        default:
        {
            return; // somebody forgot to set communication selector pin
        }
    }
    initializeDma(communicationInterface);
    startQueryingAdc();
    for (;;) ;
}
