
/**
 * Using hardware Timer interrupt to blink an LED
 */
#include <Arduino.h>

// Settings
static const uint16_t timer_divider = 80;       // 80MHz / 80 = 1MHz
static const uint32_t timer_interval = 200000; // 1MHz / 1 = 1Hz

#define LED_PIN 23U

// Globals
static hw_timer_t *timer = NULL; // Timer object

void IRAM_ATTR onTimer() // Why IRAM_ATTR? what is it? without it, it will not work?
{
    // Toggle LED state
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    timer = timerBegin(0, timer_divider, true);   // Timer 0, divider 80, count up
    timerAttachInterrupt(timer, &onTimer, true);  // Attach onTimer function// what is edge here?
    timerAlarmWrite(timer, timer_interval, true); // Set alarm every timer_interval
    timerAlarmEnable(timer);                      // Enable alarm
}

void loop()
{
    delay(10); // for simulator UI
}
