/**
 * ESP32 unbounded Priority Inversion preventation using critical section
 *
 */

#include <Arduino.h>

static const BaseType_t app_cpu = 1;

static const TickType_t cs_wait = 250;   // Time spent in critical section (ms)
static const TickType_t med_wait = 5000; // Time medium task spends working (ms)

// static SemaphoreHandle_t lock; Using spinlock/critical-section instead of a mutex lock
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED; // Spinlock

static inline TickType_t getTimestamp()
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

// Task L (low priority)
void doTaskL(void *parameters)
{
    TickType_t start_time, stop_time, timestamp;
    while (1)
    {
        // Take lock
        Serial.println("Task L trying to take lock ...");
        start_time = getTimestamp();
        // xSemaphoreTake(lock, portMAX_DELAY);
        portENTER_CRITICAL(&spinlock);

        // Say how long we spend waiting for a lock
        stop_time = getTimestamp();

        // Hog the processor for a while doing nothing (don't yeild)
        timestamp = getTimestamp();
        while (getTimestamp() - timestamp < cs_wait)
            ;
        portEXIT_CRITICAL(&spinlock);

        Serial.print("Task L got lock. Spent ");
        Serial.print(stop_time - start_time);
        Serial.println(" ms waiting for lock. Doing some work ...");
        Serial.println("Task L released the lock.");

        // Go to sleep
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// Task M (medium priority)
void doTaskM(void *parameters)
{
    TickType_t timestamp;
    while (1)
    {
        // Hog the processor for a while doing nothing
        Serial.println("Task M doing some work ...");
        timestamp = getTimestamp();
        while (getTimestamp() - timestamp < med_wait)
            ;
        // Go to sleep
        Serial.println("Task M done!");
        delay(500);
    }
}

// Task H (high priority)
void doTaskH(void *parameters)
{
    TickType_t timestamp, start_time, stop_time;
    while (1)
    {
        // Take lock
        Serial.println("Task H trying to take lock ...");
        start_time = getTimestamp();
        // xSemaphoreTake(lock, portMAX_DELAY);
        portENTER_CRITICAL(&spinlock);

        // Say how long we spend waiting for the lock
        stop_time = getTimestamp();

        // Hog the processor for a while
        timestamp = getTimestamp();
        while (getTimestamp() - timestamp < cs_wait)
            ;

        // Release lock
        // xSemaphoreGive(lock);
        portEXIT_CRITICAL(&spinlock);
        Serial.print("Task H got lock. Spent ");
        Serial.print(start_time - stop_time);
        Serial.println(" ms waiting for lock ...");
        Serial.println("Task H released lock.");

        // Go to sleep
        delay(500);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("---FreeRTOS Priority Inversion: Critical Section Solution---");

    // The order of starting the tasks matters to force priority inversion
    xTaskCreatePinnedToCore(doTaskL,
                            "Task L",
                            2048,
                            NULL,
                            1,
                            NULL,
                            app_cpu);

    // delay to force the priority inversion
    delay(1);

    xTaskCreatePinnedToCore(doTaskH,
                            "Task H",
                            2048,
                            nullptr,
                            3,
                            nullptr,
                            app_cpu);

    xTaskCreatePinnedToCore(doTaskM,
                            "Task M",
                            2048,
                            NULL,
                            2,
                            NULL,
                            app_cpu);
}

void loop()
{
    delay(10); // Give Wokwi simulator UI time
}
