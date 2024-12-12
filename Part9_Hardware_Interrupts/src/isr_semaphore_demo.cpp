/**
 * ESP32 ISR Semaphore Demo
 * 
 * Read ADC values in ISR at 1 Hz and defer printing them out in a task.
 */

#include <Arduino.h>

static const BaseType_t app_cpu = 1; // Application CPU

// Settings
static const uint16_t timer_divider = 80;           // 80MHz / 80 = 1MHz
static const uint32_t timer_max_count = 1000000;    // 1MHz / 1 = 1Hz

// Pins
static const uint8_t adc_pin = A0; // ADC pin (36)

// Globals
static hw_timer_t *timer = NULL;    // Timer object
static volatile uint32_t adc_val;
static SemaphoreHandle_t bin_sem = NULL;

void IRAM_ATTR onTimer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
    // Perform action (read ADC)
    adc_val = analogRead(adc_pin);

    // Give semaphore to tell task that data is ready
    xSemaphoreGiveFromISR(bin_sem, &xHigherPriorityTaskWoken);

    // Exit from ISR (Vanilla FreeRTOS)
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

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
        // Wait for semaphore
        xSemaphoreTake(bin_sem, portMAX_DELAY);

        // Print value
        Serial.println(adc_val);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("---FreeRTOS ISR Semaphore Demo---");

    // Create binary semaphore
    bin_sem = xSemaphoreCreateBinary();

    // Create task
    xTaskCreatePinnedToCore(printValue, "PrintValue", 2048, NULL, 1, NULL, app_cpu);

    // Set up timer
    timer = timerBegin(0, timer_divider, true);    // Timer 0, divider 80, count up
    timerAttachInterrupt(timer, &onTimer, true);   // Attach onTimer function
    timerAlarmWrite(timer, timer_max_count, true); // Set alarm every timer_max_count
    timerAlarmEnable(timer);                       // Enable alarm
}

void loop()
{
    delay(10); // for simulator UI
}