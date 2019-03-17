#pragma once

#include <stdint.h>
#include <stdatomic.h>

#define DATA_BUFFER_EMPTY_INDEXES        0b1111

#define DATA_BUFFER_LENGTH               0x03
#define DATA_BUFFER_CONSUMER_INDEX_SHIFT 0x02

#define DATA_BUFFER_INDEX_MASK                     0b11
#define DATA_BUFFER_LAST_INDEX_COMPLEMENT_MASK     (~(DATA_BUFFER_INDEX_MASK))
#define DATA_BUFFER_CONSUMER_INDEX_COMPLEMENT_MASK (~(DATA_BUFFER_INDEX_MASK << DATA_BUFFER_CONSUMER_INDEX_SHIFT))

static void consumerProducerBufferResetInterruptSafe(volatile uint32_t* bufferIndexes)
{
    atomic_store(bufferIndexes, DATA_BUFFER_EMPTY_INDEXES);
}

static uint8_t consumerProducerBufferGetConsumerIndex(volatile uint32_t* bufferIndexes)
{
    const uint32_t oldDataBufferIndexes = atomic_load(bufferIndexes);
    return (oldDataBufferIndexes >> DATA_BUFFER_CONSUMER_INDEX_SHIFT) & DATA_BUFFER_INDEX_MASK;
}

static uint8_t consumerProducerBufferGetProducerIndexInterruptSafe(volatile uint32_t* bufferIndexes)
{
    const uint32_t oldDataBufferIndexes = atomic_load(bufferIndexes);

    const uint8_t lastReadIndex = oldDataBufferIndexes & DATA_BUFFER_INDEX_MASK;
    const uint8_t consumerIndex = (oldDataBufferIndexes >> DATA_BUFFER_CONSUMER_INDEX_SHIFT) & DATA_BUFFER_INDEX_MASK;

    for (uint8_t i = 0; i < DATA_BUFFER_LENGTH; ++i)
    {
        if (i != lastReadIndex && i != consumerIndex)
        {
            return i;
        }
    }

    return DATA_BUFFER_LENGTH;
}

static bool consumerProducerBufferSetLastReadIndexInterruptSafe(volatile uint32_t* bufferIndexes, uint8_t index)
{
    if (index > DATA_BUFFER_LENGTH)
    {
        return false;
    }

    uint32_t oldDataBufferIndexes;
    uint32_t newDataBufferIndexes;

    do
    {
        oldDataBufferIndexes = atomic_load(bufferIndexes);
        newDataBufferIndexes = (oldDataBufferIndexes & DATA_BUFFER_LAST_INDEX_COMPLEMENT_MASK) | index;
    }
    while (!atomic_compare_exchange_weak(bufferIndexes, &oldDataBufferIndexes, newDataBufferIndexes));

    return true;
}

static uint8_t consumerProducerBufferMoveConsumerIndexToLastReadIndex(volatile uint32_t* bufferIndexes)
{
    const uint32_t oldDataBufferIndexes = atomic_load(bufferIndexes);
    const uint8_t lastReadIndex = oldDataBufferIndexes & DATA_BUFFER_INDEX_MASK;
    atomic_store(bufferIndexes, (oldDataBufferIndexes & DATA_BUFFER_CONSUMER_INDEX_COMPLEMENT_MASK) | (lastReadIndex << DATA_BUFFER_CONSUMER_INDEX_SHIFT));
    return lastReadIndex;
}
