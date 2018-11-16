/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "imu.h"
#include "spi.h"
#include "clock.h"
#include "global_data.h"

#include "imu_lsm6ds3h.h"

void initializeImuLsm6ds3h()
{
    setSpiBuffers(&g_imuRxBuffer, g_imuTxBuffer);

    g_imuTxBuffer[0]  = 0b00010000; // CTRL1_XL (10h): address
    g_imuTxBuffer[1]  = 0b10000011; // CTRL1_XL (10h): value: +/- 2g, 1.66 kHz, 50Hz filter bandwidth
    g_imuTxBuffer[2]  = 0b10000100; // CTRL2_G  (11h): value: 500 dps, 1.66 kHz
    g_imuTxBuffer[3]  = 0b01000110; // CTRL3_C  (12h): value: BDU=1, IF_INC=1, BLE=1
    g_imuTxBuffer[4]  = 0b10000000; // CTRL4_C  (13h): value: XL_BW_SCAL_ODR=1
    g_imuTxBuffer[5]  = 0b00000000; // CTRL5_C  (14h): value:
    g_imuTxBuffer[6]  = 0b00000000; // CTRL6_C  (15h): value:
    g_imuTxBuffer[7]  = 0b00000000; // CTRL7_G  (16h): value:
    g_imuTxBuffer[8]  = 0b11100001; // CTRL8_XL (17h): value: LPF2_XL_EN=1, LOW_PASS_ON_6D=1, LP Filter cutoff frequency=ODR_XL/400
    g_imuTxBuffer[9]  = 0b00111000; // CTRL9_XL (18h): value: enable = x, y, z
    g_imuTxBuffer[10] = 0b00111100; // CTRL10_C (19h): value: enable = yaw, pitch, roll

    // wait for imu device to boot (boot start time is time when devices received power)
    while(getCurrentTimeInMicroseconds() < 20 * 1000) ;

    startTransmitReceiveSpi(11, SPI_DEVICE_IMU);

    while (isSpiBusy()) ;

    // wait for imu gyroscope to start
    uint32_t imuGyroscopeBootStartTime = getCurrentTimeInMicroseconds();
    while (getDifferenceWithCurrentTime(imuGyroscopeBootStartTime) < 80 * 1000) ;
}

void startQueryingImuLsm6ds3h()
{
    g_imuTxBuffer[0] = 0b10011110; // STATUS_REG(1Eh): address
    g_imuTxBuffer[1] = 0b00000000; // STATUS_REG(1Eh): read placeholder
    g_imuTxBuffer[2] = 0b00000000; // OUT_TEMP_L(20h): read placeholder
    g_imuTxBuffer[3] = 0b00000000; // OUT_TEMP_h(21h): read placeholder
    g_imuTxBuffer[4] = 0b00000000; // OUTX_L_G  (22h): read placeholder
    g_imuTxBuffer[5] = 0b00000000; // OUTX_H_G  (23h): read placeholder
    g_imuTxBuffer[6] = 0b00000000; // OUTY_L_G  (24h): read placeholder
    g_imuTxBuffer[7] = 0b00000000; // OUTY_H_G  (25h): read placeholder
    g_imuTxBuffer[8] = 0b00000000; // OUTZ_L_G  (26h): read placeholder
    g_imuTxBuffer[9] = 0b00000000; // OUTZ_H_G  (27h): read placeholder

    startTransmitReceiveSpi(10, SPI_DEVICE_IMU);
}

float getImuYawAngleRadians()
{
    // normalized_value = read_value * 17.50f / 1000.0f * 2.0f * pi / 180.0f
    // yaw_angle_radians = normalized_value * time_delta_since_last_read
    return 0.0f;
}
