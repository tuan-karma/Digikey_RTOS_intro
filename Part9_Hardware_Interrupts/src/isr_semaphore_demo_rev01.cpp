/**
 * ESP32 ISR Semaphore Demo
 *
 * Read ADC values in ISR at 1 Hz and defer printing them out in a task.
 * rev01:
 * - Using periodicHWTimer in the utilities.hpp
 * - Using taskNotification instead of Semaphore
 */

#include <Arduino.h>
#include "utilities.hpp"

static const BaseType_t app_cpu = 1; // Application CPU
static const uint8_t adc_pin = A0;   // ADC pin (36)

// Globals
static volatile uint32_t adc_val;
static TaskHandle_t print_task = NULL;

void IRAM_ATTR onTimer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Perform action (read ADC)
    adc_val = analogRead(adc_pin);

    vTaskNotifyGiveFromISR(print_task, &xHigherPriorityTaskWoken);
    // Exit from ISR (ESP-IDF)
    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

void printValue(void *pvParameters)
{
    while (1)
    {
        // Wait for notification from ISR
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Print value
        Serial.println(adc_val);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("---FreeRTOS ISR Semaphore Demo---");
    // Create task
    xTaskCreatePinnedToCore(printValue,
                            "PrintValue",
                            2048,
                            NULL,
                            2,
                            &print_task,
                            app_cpu);

    // Set up timer
    periodicHWTimer(0, 1000, &onTimer);
}

void loop()
{
    delay(10); // for simulator UI
}