#include <Arduino.h>
#include <utilities.hpp>

enum
{
    LED1 = 22,
    LED2 = 23,
};

void IRAM_ATTR onTimer1()
{
    digitalWrite(LED1, !digitalRead(LED1));
}

void IRAM_ATTR onTimer2()
{
    digitalWrite(LED2, !digitalRead(LED2));
}

void setup()
{
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    periodicHWTimer(0, 500, &onTimer1);
    periodicHWTimer(3, 501, &onTimer2);
}

void loop()
{
    delay(10);
}