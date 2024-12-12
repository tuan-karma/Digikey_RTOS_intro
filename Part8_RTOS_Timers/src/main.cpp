/**
 * Exercise: simulate the dimming function of the LCD backlight
 */

#include <Arduino.h>
#define LCD_BACKLIGHT_PIN 23
#define BACKLIGHT_TIMEOUT_MS 5000

TimerHandle_t backlight_timer; // one-short

void backlight_timer_callback(TimerHandle_t xTimer)
{
    digitalWrite(LCD_BACKLIGHT_PIN, LOW); // turn off the backlight
}

void uartCLI(void * pvParameters)
{
    while (1)
    {
        if (Serial.available() > 0) {
            char c = Serial.read();
            Serial.print(c); // just echo back
            digitalWrite(LCD_BACKLIGHT_PIN, HIGH); // turn on the backlight
            xTimerStart(backlight_timer, portMAX_DELAY);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000); // for UART connection
    Serial.println();
    Serial.println("---LCD Backlight Dimming Exercise---");

    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(LCD_BACKLIGHT_PIN, HIGH); // turn on the backlight

    backlight_timer = xTimerCreate("Backlight timer",
                                   pdMS_TO_TICKS(BACKLIGHT_TIMEOUT_MS),
                                   pdFALSE,   // auto-reload = false (one-shot)
                                   (void *)0, // timer ID
                                   backlight_timer_callback);

    if (backlight_timer != nullptr)
        xTimerStart(backlight_timer, portMAX_DELAY);
    
    xTaskCreate(uartCLI, "uartCLI", 2048, NULL, 1, NULL);
}

void loop()
{
    delay(1000); // for the simulator UI
}