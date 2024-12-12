/**
 * ESP32 Sample and Process Solution
 *
 * Sample ADC in an ISR, process in a task.
 *
 * Date: February 3, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */
#include <Arduino.h>
static const BaseType_t app_cpu = 1;
#include "utilities.hpp"

// Settings
static const uint32_t cli_delay = 1000; // ms delay
static const uint8_t adc_pin = A0;     // GPIO 36U

enum
{
    BUF_LEN = 10,      // Sample buffer len
    MSG_LEN = 100,     // Max characters in message
    ERR_QUEUE_LEN = 5, // Number of slots in message queue
};

// Message struct to wrap strings for queue
struct Message
{
    char body[MSG_LEN];
};

// Globals
static hw_timer_t *timer;
static TaskHandle_t processing_task = NULL;
static SemaphoreHandle_t sem_done_reading = NULL;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t err_msg_queue;
static volatile uint16_t buf_0[BUF_LEN];     // One buffer in the pair
static volatile uint16_t buf_1[BUF_LEN];     // The other buffer in the pair
static volatile uint16_t *write_to = buf_0;  // Double buffer write pointer
static volatile uint16_t *read_from = buf_1; // Double buffer read pointer
static volatile uint8_t buf_overrun = 0;     // Double buffer overrun flag
static float adc_avg;

//*****************************************************************************
// Functions that can be called from anywhere (in this file)

// Swap the write_to and read_from pointers in the double buffer
// Only ISR calls this at the moment, so no need to make it thread-safe

//*****************************************************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer()
{
    static uint16_t idx = 0;
    BaseType_t task_woken = pdFALSE;

    // If buffer is not overrun, read ADC to next buffer element. If buffer is
    // overrun, drop the sample.
    if ((idx < BUF_LEN) && (buf_overrun == 0))
    {
        write_to[idx] = analogRead(adc_pin);
        idx++;
    }

    // Check if the buffer is full
    if (idx >= BUF_LEN)
    {
        // If reading is not done, set overrun flag. We don't need to set this
        // as a critical section, as nothing can interrupt and change either value.
        if (xSemaphoreTakeFromISR(sem_done_reading, &task_woken) == pdFALSE)
        {
            buf_overrun = 1;
        }

        // Only swap buffers and notify task if overrun flag is cleared
        if (buf_overrun == 0)
        {
            // Reset index and swap buffer pointers
            idx = 0;
            std::swap(write_to, read_from);
            // A task notification works like a binary semaphore but is faster
            vTaskNotifyGiveFromISR(processing_task, &task_woken); // deferring to the processing_task
        }
    }
    // Exit from ISR (ESP-IDF)
    if (task_woken)
    {
        portYIELD_FROM_ISR();
    }
}

//*****************************************************************************
// Tasks

// Serial terminal task
void doCLI(void *parameters)
{
    Message err_msg;

    while (1)
    {
        // Looking for any error messages that need to be printed
        if (xQueueReceive(err_msg_queue, (void *)&err_msg, 0) == pdTRUE)
        {
            Serial.println(err_msg.body);
        }
        Serial.print("Average: ");
        Serial.println(adc_avg);
        vTaskDelay(cli_delay / portTICK_PERIOD_MS);
    }
}

// Wait for semaphore and calculate average of ADC values
void calcAverage(void *parameters)
{
    Message msg;
    float avg;

    // Loop forever, wait for semaphore, and print value
    while (1)
    {
        // Wait for notification from ISR (similar to binary semaphore)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        avg = average((uint16_t *)read_from, BUF_LEN); // processing can take long time in practice --> buffer overrun
        // Updating the shared float may or may not take multiple instructions, so
        // we protect it with a mutex or critical section. The ESP-IDF critical
        // section is the easiest for this application.
        portENTER_CRITICAL(&spinlock);
        adc_avg = avg;
        portEXIT_CRITICAL(&spinlock);

        // If we took too long to process, buffer writing will have overrun. So,
        // we send a message to be printed out to the serial terminal.
        if (buf_overrun == 1)
        {
            strcpy(msg.body, "Error: Buffer overrun. Samples have been dropped.");
            xQueueSend(err_msg_queue, (void *)&msg, 10);
        }

        // Clearing the overrun flag and giving the "done reading" semaphore must
        // be done together without being interrupted.
        portENTER_CRITICAL(&spinlock);
        buf_overrun = 0;
        xSemaphoreGive(sem_done_reading);
        portEXIT_CRITICAL(&spinlock);
    }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup()
{
    // Configure Serial
    Serial.begin(115200);
    // Wait a moment to start (so we don't miss Serial output)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println();
    Serial.println("---FreeRTOS Sample and Process Demo---");

    // Create semaphore before it is used (in task or ISR)
    sem_done_reading = xSemaphoreCreateBinary();
    // Force reboot if we can't create the semaphore
    if (sem_done_reading == NULL)
    {
        Serial.println("Could not create one or more semaphores");
        ESP.restart();
    }

    // We want the done reading semaphore to initialize to 1
    xSemaphoreGive(sem_done_reading);

    // Create message queue before it is used
    err_msg_queue = xQueueCreate(ERR_QUEUE_LEN, sizeof(Message));

    // Start task to handle command line interface events. Let's set it at a
    // higher priority but only run it once every 20 ms.
    xTaskCreatePinnedToCore(doCLI,
                            "Do CLI",
                            2048,
                            NULL,
                            2,
                            NULL,
                            app_cpu);

    // Start task to calculate average. Save handle for use with notifications.
    xTaskCreatePinnedToCore(calcAverage,
                            "Calculate average",
                            1024,
                            NULL,
                            1,
                            &processing_task,
                            app_cpu);

    // Start a timer to run ISR every 100 ms
    timer = periodicHWTimer(0, 100, &onTimer);
}

void loop()
{
    // Do nothing
    vTaskDelay(10 / portTICK_PERIOD_MS); // for simulator UI
}