
/**
 * ESP32 ISR Critical Section Demo
 *
 * Increment global variable in ISR.
 * Date: Feb. 3, 2021
 * Author: Shawn Hymel
 * https://github.com/ShawnHymel/introduction-to-rtos/blob/main/09-hardware-interrupts/esp32-freertos-09-demo-isr-critical-section/esp32-freertos-09-demo-isr-critical-section.ino
 *
 */

#include <Arduino.h>

static const BaseType_t app_cpu = 1; // Application CPU

// Settings
static const uint16_t timer_divider = 8;                        // 80MHz / 8 = 10MHz
static const uint32_t timer_max_count = 1000000;                // 10MHz / 1 = 10Hz
static const TickType_t task_delay = 2000 / portTICK_PERIOD_MS; // 2000 ms

// Globals
static hw_timer_t *timer = NULL;                             // Timer object
static volatile uint32_t isr_counter;                        // Counter incremented in ISR
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED; // Spinlock

void IRAM_ATTR onTimer()
{
    // Increment counter
    portENTER_CRITICAL_ISR(&spinlock);
    isr_counter++;
    portEXIT_CRITICAL_ISR(&spinlock);
}

void printValue(void *pvParameters)
{
    while (1)
    {
        while (isr_counter > 0)
        {
            Serial.println(isr_counter);

            portENTER_CRITICAL(&spinlock);
            isr_counter--;
            portEXIT_CRITICAL(&spinlock);
        }
        Serial.println("ISR incrementing counter...");
        vTaskDelay(task_delay); // wait 2 seconds while ISR increments counter a few times
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("---FreeRTOS ISR Critical Section Demo---");

    xTaskCreatePinnedToCore(printValue, "PrintValue", 2048, NULL, 1, NULL, app_cpu);
    timer = timerBegin(0, timer_divider, true);    // Timer 0, divider 8, count up
    timerAttachInterrupt(timer, &onTimer, true);   // Attach onTimer function
    timerAlarmWrite(timer, timer_max_count, true); // Set alarm every timer_max_count
    timerAlarmEnable(timer);                       // Enable alarm
}

void loop()
{
    delay(10); // for simulator UI
}