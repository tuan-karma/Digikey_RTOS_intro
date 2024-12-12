// From the Shawn Hymel FreeRTOS lecture: https://youtu.be/pHJ3lxOoWeI?si=4dXUyL12yM4SI_bD
#include <Arduino.h>

static const BaseType_t app_cpu = 1;

static const uint8_t msg_queue_len = 5;
// Globals
static QueueHandle_t msg_queue;

// Task: wait for item on queue and print it out
void printMessages(void *pargs)
{
    int item;
    while (1)
    {
        if (xQueueReceive(msg_queue, (void *)&item, 0) == pdTRUE)
        {
            // Serial.println(item);
        }
        Serial.println(item);
        delay(1000);
    }
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("---FreeRTOS Queue Demo---");

    msg_queue = xQueueCreate(msg_queue_len, sizeof(int));

    xTaskCreatePinnedToCore(printMessages,
                            "Print Messages",
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
}

void loop()
{
    // put your main code here, to run repeatedly:
    static int num = 0;
    if (xQueueSend(msg_queue, (void *)&num, 10) != pdTRUE)
    {
        Serial.println("Queue full");
    }
    num++;

    delay(500); // this speeds up the simulation
}
