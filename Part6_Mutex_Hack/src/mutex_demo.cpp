
// #include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

SemaphoreHandle_t xMutex; // Khai báo mutex

void task1(void *pvParameters) {
    while (1) {
        // Yêu cầu sở hữu mutex
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
            // Bắt đầu critical section
            // ... Thao tác với tài nguyên được chia sẻ ...
            // Kết thúc critical section
            xSemaphoreGive(xMutex); // Giải phóng mutex
        }
    }
}