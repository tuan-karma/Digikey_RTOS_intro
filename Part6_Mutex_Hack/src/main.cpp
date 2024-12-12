#include <Arduino.h>

static const BaseType_t app_cpu = 1;

SemaphoreHandle_t bin_sem;

void blinkLED(void *pargs)
{
    uint32_t delay_arg = *(uint32_t *)pargs;
    xSemaphoreGive(bin_sem);

    Serial.print("Recieved delay: ");
    Serial.println(delay_arg);
    pinMode(22, OUTPUT);
    while (1)
    {
        digitalWrite(22, HIGH);
        delay(delay_arg);
        digitalWrite(22, LOW);
        delay(delay_arg);
    }
}

void setup()
{
    bin_sem = xSemaphoreCreateBinary();

    uint32_t delay_arg;
    Serial.begin(115200);
    delay(10);
    Serial.println("---Mutex Hack Challenge---");
    Serial.println("Enter a number to delay the LED blink in milliseconds");

    while (Serial.available() == 0)
    {
        delay(10);
    }

    delay_arg = Serial.parseInt();
    Serial.print("Delay: ");
    Serial.println(delay_arg);

    // Start the LED task
    xTaskCreatePinnedToCore(blinkLED,
                            "Blink LED",
                            2048,
                            (void *)&delay_arg,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);

    Serial.println("Done!");
}

void loop()
{
    delay(10); // to accerlate the simulation
}