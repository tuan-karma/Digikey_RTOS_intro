/**
 * Software timers in FreeRTOS on ESP32 demonstration
 *
 */
#include <Arduino.h>
TimerHandle_t one_shot_timer;
TimerHandle_t periodic_timer;

void timerCallback(TimerHandle_t xTimer)
{
    int timer_id = (int)pvTimerGetTimerID(xTimer);
    if (timer_id == 0)
        Serial.println("Timer expired");
    else if (timer_id == 1)
        Serial.println("Periodic timer expired");
    else
        Serial.println("Wrong timer ID");
}

void setup()
{
    Serial.begin(115200);
    delay(1000); // for UART connection
    Serial.println();
    Serial.println("---FreeRTOS Software Timer Demo---");

    // Create a one-shot timer
    one_shot_timer = xTimerCreate("One-shot timer",
                                  pdMS_TO_TICKS(1000),
                                  pdFALSE,   // auto-reload = false (one-shot)
                                  (void *)0, // timer ID
                                  timerCallback);

    if (one_shot_timer != nullptr)
        xTimerStart(one_shot_timer, 0);
    
    // Create a periodic timer
    periodic_timer = xTimerCreate("Periodic timer",
                                  pdMS_TO_TICKS(1000),
                                  pdTRUE,    // auto-reload = true (periodic)
                                  (void *)1, // timer ID
                                  timerCallback);
    
    if (periodic_timer != nullptr)
        xTimerStart(periodic_timer, 0);
}

void loop()
{
    delay(1000); // for the simulator UI
}