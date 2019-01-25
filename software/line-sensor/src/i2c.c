#include "i2c.h"

#include <stm32/stm32l1xx_ll_bus.h>
#include <stm32/stm32l1xx_ll_i2c.h>

void initializeI2c()
{
    LL_I2C_Disable(I2C1);
    LL_I2C_SetOwnAddress1(I2C1, I2C_SLAVE_ADDRESS, LL_I2C_OWNADDRESS1_7BIT);

    LL_I2C_EnableDMAReq_RX(I2C1);
    LL_I2C_EnableDMAReq_TX(I2C1);
}
