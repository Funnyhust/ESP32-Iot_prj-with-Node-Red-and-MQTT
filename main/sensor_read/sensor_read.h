#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "aht20/aht20.h"
#include "bmp280/bmp280_read.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h> // Thêm include cho SemaphoreHandle_t

// Định nghĩa kiểu sensor_data_t
typedef struct {
    float temperature;
    float humidity;
    float pressure;
} sensor_data_t;

// Khai báo biến toàn cục
extern sensor_data_t sensor_data;
extern SemaphoreHandle_t sensor_data_mutex;

// Khai báo các task
void read_sensor_task(void *pvParameters);
void warning_task(void *pvParameters);

#endif // SENSOR_READ_H