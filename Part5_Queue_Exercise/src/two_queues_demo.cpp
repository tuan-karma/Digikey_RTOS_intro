/**
 * Demonstration of two queues to implement a full duplex communication between two tasks.
 */
#include <Arduino.h>

// Define the queues
QueueHandle_t queue1;
QueueHandle_t queue2;

// The serialMutex
SemaphoreHandle_t serialMutex;

// Task 1: Send data to queue1 and receive data from queue2
void Task1(void *pvParameters)
{
    int sendData = 0;
    int receiveData = 0;

    while (1)
    {
        // Send data to queue1
        xQueueSend(queue1, &sendData, portMAX_DELAY);
        sendData++;

        // Receive data from queue2
        if (xQueueReceive(queue2, &receiveData, 0) == pdTRUE)
        {
            xSemaphoreTake(serialMutex, portMAX_DELAY);
            Serial.print("Task1 received: ");
            Serial.println(receiveData);
            xSemaphoreGive(serialMutex);
        }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        delay(200);
    }
}

// Task 2: Send data to queue2 and receive data from queue1
void Task2(void *pvParameters)
{
    int sendData = 100;
    int receiveData = 0;

    while (1)
    {
        // Send data to queue2
        xQueueSend(queue2, &sendData, portMAX_DELAY);
        sendData++;

        // Receive data from queue1
        if (xQueueReceive(queue1, &receiveData, 0) == pdTRUE)
        {
            xSemaphoreTake(serialMutex, portMAX_DELAY);
            Serial.print("Task2 received: ");
            Serial.println(receiveData);
            xSemaphoreGive(serialMutex);
        }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        delay(200);
    }
}

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    delay(10);
    // Create the queues
    queue1 = xQueueCreate(10, sizeof(int));
    queue2 = xQueueCreate(10, sizeof(int));

    serialMutex = xSemaphoreCreateMutex();

    // Create and start the tasks
    xTaskCreate(Task1, "Task1", 1024, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", 1024, NULL, 1, NULL);
}

void loop()
{
    // Empty. Things are done in Tasks.
    delay(10); // this speeds up the simulation
}