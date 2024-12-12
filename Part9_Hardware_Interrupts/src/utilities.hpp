#include <Arduino.h>

// Swap the write_to and read_from pointers in the double buffer
// Only ISR calls this at the moment, so no need to make it thread-safe
inline void test_swap()
{
    int a = 0, b = 22;
    int *pa = &a;
    int *pb = &b;
    std::swap(pa, pb);
    assert(*pa == 22);
    assert(*pb == 0);
}

inline float average(uint16_t *buf, int len)
{
    float sum = 0.0;
    for (int i = 0; i < len; i++)
    {
        sum += (float)buf[i];
        // vTaskDelay(pdMS_TO_TICKS(105)); // simulate the processing workload --> test overrun flag
    }
    return sum / len;
}

inline void test_average()
{
    uint16_t test_array[]{1, 2, 3};
    volatile uint16_t *read = test_array;
    float expected_avg = 2.0;
    float avg = average((uint16_t *)read, 3);
    assert(avg == expected_avg);
}

inline hw_timer_t *periodicHWTimer(uint8_t number, uint64_t ms, void (*isrCallback)())
{
    uint64_t max_count = 1000 * ms;
    hw_timer_t *timer = timerBegin(number, 80, true); // 80 / 80 = 1 MHz
    timerAttachInterrupt(timer, isrCallback, true);
    timerAlarmWrite(timer, max_count, true);
    timerAlarmEnable(timer);
    return timer;
}