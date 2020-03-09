import math

prescaler = 1
cpuFrequency = 80_000_000

minFrequency = 100
maxFrequency = 1500
rampUpTimeSeconds = 1

def f(minFrequency, maxFrequency, rampUpTimeSeconds, x):
    return minFrequency + (maxFrequency - minFrequency) - (maxFrequency - minFrequency) * math.cos(2 * math.pi * x / (4 * rampUpTimeSeconds))

def frequencyToArr(cpuFrequency, prescaler, x):
    return int(cpuFrequency / (x * (prescaler + 1)) - 1)

totalTime = 0
stepsCount = 303

minArr = frequencyToArr(cpuFrequency, prescaler, minFrequency)
maxArr = frequencyToArr(cpuFrequency, prescaler, maxFrequency)

for i in range(0, stepsCount + 1):
    x = rampUpTimeSeconds / stepsCount * i
    fx = f(minFrequency, maxFrequency, rampUpTimeSeconds, x)
    t = 1 / fx
    totalTime += t

print(f'#include "global_data_timer.h"')
print()
print(f'#define DMA_TIMER_STEPS_COUNT       {stepsCount}')
print()
print(f'#define DMA_TIMER_MIN_FREQUENCY_ARR {minArr}')
print(f'#define DMA_TIMER_MIN_FREQUENCY_CC  {minArr >> 1}')
print()
print(f'#define DMA_TIMER_MAX_FREQUENCY_ARR {maxArr}')
print(f'#define DMA_TIMER_MAX_FREQUENCY_CC  {maxArr >> 1}')
print()
print(f'const uint32_t g_dmaTimerDataIncreasing[(DMA_TIMER_STEPS_COUNT + 1) * 3] = {{')
for i in range(1, stepsCount + 1):
    x = rampUpTimeSeconds / stepsCount * i
    fx = f(minFrequency, maxFrequency, rampUpTimeSeconds, x)
    arr = frequencyToArr(cpuFrequency, prescaler, fx)
    print(f'    {arr},')
    print(f'    0,')
    print(f'    {arr >> 1},')
print(f'    0,')
print(f'    0,')
print(f'    0,')
print('};')
print()
print(f'const uint32_t g_dmaTimerDataDecreasing[(DMA_TIMER_STEPS_COUNT + 1) * 3] = {{')
for i in range(1, stepsCount + 1):
    x = rampUpTimeSeconds - rampUpTimeSeconds / stepsCount * i
    fx = f(minFrequency, maxFrequency, rampUpTimeSeconds, x)
    arr = frequencyToArr(cpuFrequency, prescaler, fx)
    print(f'    {arr},')
    print(f'    0,')
    print(f'    {arr >> 1},')
print(f'    0,')
print(f'    0,')
print(f'    0,')
print('};')
print()
print(f'// start time = {totalTime}')
