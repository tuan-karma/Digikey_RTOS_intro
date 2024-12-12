/**
 * Solution to 05 - Queue Challenge
 *
 * One task performs basic echo on Serial. If it sees "delay" followed by a
 * number, it sends the number (in a queue) to the second task. If it receives
 * a message in a second queue, it prints it to the console. The second task
 * blinks an LED. When it gets a message from the first queue (number), it
 * updates the blink delay to that number. Whenever the LED blinks 100 times,
 * the second task sends a message to the first task to be printed.
 *
 * Date: January 18, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */
#include <Arduino.h>
// Use only core 1 for demo purposes
static const BaseType_t app_cpu = 1;

// Settings
static const uint8_t buf_len = 255;       // Size of buffer to look for commands
static const char command[] = "delay ";   // Command to look for. Note the space!
static const uint8_t delay_queue_len = 5; // Size of delay queue
static const uint8_t msg_queue_len = 5;   // Size of message queue
static const uint8_t blink_max = 10;      // Number of blinks before sending message

static const int led_pin = 22; // Pin for the Green LED

// Message struct: used to wrap strings (not necessary, but it's useful to see how to use structs here)
typedef struct Message
{
    char body[20];
    int count;
} Message;

// Two Globals Queues
static QueueHandle_t msg_queue;
static QueueHandle_t delay_queue;

// Task: CLI
void doCLI(void *pargs)
{
    Message rcv_msg;
    char c;
    char buf[buf_len];
    uint8_t idx = 0;
    uint8_t cmd_len = strlen(command);
    int led_delay;

    // Clear whole buffer
    memset(buf, 0, buf_len);

    while (1)
    {
        // See if there's a message in the queue (do not block)
        if (xQueueReceive(msg_queue, (void *)&rcv_msg, 0) == pdTRUE)
        {
            Serial.print(rcv_msg.body);
            Serial.println(rcv_msg.count);
        }

        // Read characters from serial
        if (Serial.available() > 0)
        {
            c = Serial.read();

            // Store received character to buffer if not over buffer limit
            if (idx < buf_len - 1)
            {
                buf[idx] = c;
                idx++;
            }
            else
            {
                memset(buf, 0, buf_len);
                idx = 0;
            }

            // Print newline and check input on 'enter'
            if ((c == '\n') || (c == '\r'))
            {
                // Print newline to terminal
                Serial.print("\r\n");
                // Check if the first 6 characters are "delay "
                if (memcmp(buf, command, cmd_len) == 0)
                {
                    // Convert last part to positive integer (negative int crashes)
                    char *tail = buf + cmd_len;
                    led_delay = atoi(tail);
                    led_delay = abs(led_delay);

                    // Send integer to other task via queue
                    if (xQueueSend(delay_queue, (void *)&led_delay, 10) != pdTRUE)
                    {
                        Serial.println("ERROR: Could not put item on delay queue.");
                    }
                }
                // Reset receive buffer and index counter
                memset(buf, 0, buf_len);
                idx = 0;
                // Otherwise, echo character back to serial terminal
            }
            else
            {
                Serial.print(c); // echo back to the serial
            } // end if (c == '\n')
            // yeild to other tasks
            delay(10);
        } // end if (Serial.available() > 0)
    }
}

void blinkLED(void *pargs)
{
    Message msg;
    int led_delay = 500;
    uint8_t counter = 0;

    // Set up LED
    pinMode(led_pin, OUTPUT);
    while (1)
    {
        if (xQueueReceive(delay_queue, (void *)&led_delay, 0) == pdTRUE)
        {
            strcpy(msg.body, "Message received ");
            msg.count = 1;
            xQueueSend(msg_queue, (void *)&msg, 10);
        }

        // Blink
        digitalWrite(led_pin, HIGH);
        delay(led_delay);
        digitalWrite(led_pin, LOW);
        delay(led_delay);

        counter++;
        if (counter >= blink_max)
        {
            strcpy(msg.body, "Blinked: ");
            msg.count = counter;
            xQueueSend(msg_queue, (void *)&msg, 10);
            counter = 0;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    Serial.println("---FreeRTOS double Queue Challenge---");
    Serial.println("Enter the command 'delay <number>' to change the LED blink delay in milliseconds.");

    delay_queue = xQueueCreate(delay_queue_len, sizeof(int));
    msg_queue = xQueueCreate(msg_queue_len, sizeof(Message));

    // Start CLI task
    xTaskCreatePinnedToCore(doCLI,
                            "CLI",
                            2048,
                            NULL,
                            1,
                            NULL,
                            app_cpu);

    // Start LED task
    xTaskCreatePinnedToCore(blinkLED,
                            "Blink LED",
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
}

void loop()
{
    // Empty loop
    delay(10); // this speeds up the simulation
}