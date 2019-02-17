#pragma once

#include "types.h"
#include "global_data.h"

#include <stdatomic.h>

static bufferIndex_t getCurrentProducerIndex()
{
    return (bufferIndex_t) (atomic_load(&g_currentProducerConsumerIndex) & 0x0000FFFF);
}

static void moveProducerIndexToNextOne()
{
    uint32_t oldValue;
    uint32_t newValue;

    do
    {
        oldValue = atomic_load(&g_currentProducerConsumerIndex);

        uint32_t producerIndex = oldValue & 0x0000FFFF;
        uint32_t consumerIndex = (oldValue & 0xFFFF0000) >> 16;

        do
        {
            ++producerIndex;

            if (producerIndex >= NUMBER_OF_DATA_BUFFERS)
            {
                producerIndex = 0;
            }
        } while (producerIndex == consumerIndex);

        newValue = producerIndex | (consumerIndex << 16);

    } while (!atomic_compare_exchange_weak(&g_currentProducerConsumerIndex, &oldValue, newValue));
}
