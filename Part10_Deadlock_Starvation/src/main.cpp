/**
 * ESP32 Dining Philosophers
 *
 * The classic "Dining Philosophers" problem in FreeRTOS form.
 *
 * https://www.youtube.com/watch?v=hRsWi4HIENc
 */
#include <Arduino.h>

static const BaseType_t app_cpu = 1;
enum
{
    NUM_TASKS = 5, // number of tasks (philosophers)
    TASK_STACK_SIZE = 2048
};

static SemaphoreHandle_t bin_sem;              // wait for parameters to be read
static SemaphoreHandle_t done_sem;             // notifies main task when done as counting semaphores starts at 0
static SemaphoreHandle_t chopstick[NUM_TASKS]; // as mutexes took guard the shared resource (the noodle bowl)

// Tasks: the only task is eating
void eat(void *parameters)
{
    int num; // the philosopher number/identifier

    // Copy parameter and increment semaphore count
    num = *(int *)parameters;
    xSemaphoreGive(bin_sem); // to signify the task creation done.

    // Take left chopstick
    xSemaphoreTake(chopstick[num], portMAX_DELAY);
    Serial.printf("Philosopher %i took chopstick %i\r\n", num, num);

    // Add some delay to force deadlock
    delay(2);

    // Take right chopstick
    xSemaphoreTake(chopstick[(num + 1) % NUM_TASKS], portMAX_DELAY);
    Serial.printf("Philosopher %i took chopstick %i\r\n", num, (num + 1) % NUM_TASKS);

    // Do some eating
    Serial.printf("Philoshoper %i is eating\r\n", num);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Put down right chopstick
    xSemaphoreGive(chopstick[(num + 1) % NUM_TASKS]);
    Serial.printf("Philosopher %i returned chopstick %i\r\n", num, (num + 1) % NUM_TASKS);

    // Put down left chopstick
    xSemaphoreGive(chopstick[num]);
    Serial.printf("Philosopher %i returned chopstick %i\r\n", num, num);

    // Notify main task and delete self
    xSemaphoreGive(done_sem); // increase the done_sem counting semaphore
    vTaskDelete(NULL);
}

// Main (runs as its own task with priority 1 on core 1 - app_cpu)

void setup()
{
    char task_name[20];
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("---FreeRTOS Dining Philosophers Challenge---");

    // Create kernel objects before starting tasks
    bin_sem = xSemaphoreCreateBinary();
    done_sem = xSemaphoreCreateCounting(NUM_TASKS, 0);
    for (int i = 0; i < NUM_TASKS; i++)
    {
        chopstick[i] = xSemaphoreCreateMutex();
    }

    // Have the philosophers start eating
    for (int i = 0; i < NUM_TASKS; i++)
    {
        sprintf(task_name, "Philosopher %i", i);
        xTaskCreatePinnedToCore(eat,
                                task_name,
                                TASK_STACK_SIZE,
                                (void *)&i,
                                1,
                                NULL,
                                app_cpu);
        xSemaphoreTake(bin_sem, portMAX_DELAY); // ensure the task was created and run before creating next task
    }

    // Wait until all the philosophers are done
    for (int i = 0; i < NUM_TASKS; i++)
    {
        xSemaphoreTake(done_sem, portMAX_DELAY);
    }

    // Say that we made it through without deadlock
    Serial.println("Done! No deadlock occurred!");
}

void loop()
{
    delay(10); // Give time to the Wokwi simulator UI
}