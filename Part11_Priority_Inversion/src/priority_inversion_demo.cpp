/**
 * ESP32 Priority Inversion Demo
 *
 */

#include <Arduino.h>

static const BaseType_t app_cpu = 1;

TickType_t cs_wait = 250;   // Time spent in critical section (ms)
TickType_t med_wait = 5000; // Time medium task spends working (ms)

static SemaphoreHandle_t lock;

static inline TickType_t getTimestamp()
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

// Task L (low priority)
void doTaskL(void *parameters)
{
    TickType_t timestamp;
    while (1)
    {
        // Take lock
        Serial.println("Task L trying to take lock ...");
        timestamp = getTimestamp();
        xSemaphoreTake(lock, portMAX_DELAY);

        // Say how long we spend waiting for a lock
        Serial.print("Task L got lock. Spent ");
        Serial.print(getTimestamp() - timestamp);
        Serial.println(" ms waiting for lock. Doing some work ...");

        // Hog the processor for a while doing nothing (don't yeild)
        timestamp = getTimestamp();
        while (getTimestamp() - timestamp < cs_wait)
            ;

        // Release lock
        Serial.println("Task L releasing lock.");
        xSemaphoreGive(lock);

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
    TickType_t timestamp;
    while (1)
    {
        // Take lock
        Serial.println("Task H trying to take lock ...");
        timestamp = getTimestamp();
        xSemaphoreTake(lock, portMAX_DELAY);

        // Say how long we spend waiting for the lock
        Serial.print("Task H got lock. Spent ");
        Serial.print(getTimestamp() - timestamp);
        Serial.println(" ms waiting for lock. Doing some work ...");

        // Hog the processor for a while
        timestamp = getTimestamp();
        while (getTimestamp() - timestamp < cs_wait)
            ;

        // Release lock
        Serial.println("Task H releasing lock.");
        xSemaphoreGive(lock);

        // Go to sleep
        delay(500);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("---FreeRTOS Priority Inversion Demo---");

    lock = xSemaphoreCreateBinary(); // Note: not a mutex!
    xSemaphoreGive(lock);            // Make sure binary semaphore starts at 1

    // The order of starting the tasks matters to force priority inversion
    xTaskCreatePinnedToCore(doTaskL,
                            "Task L",
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);

    // delay to force the priority inversion
    delay(1);

    xTaskCreatePinnedToCore(doTaskH,
                            "Task H",
                            1024,
                            nullptr,
                            3,
                            nullptr,
                            app_cpu);

    xTaskCreatePinnedToCore(doTaskM,
                            "Task M",
                            1024,
                            NULL,
                            2,
                            NULL,
                            app_cpu);
}

void loop()
{
    delay(10); // Give Wokwi simulator UI time
}
