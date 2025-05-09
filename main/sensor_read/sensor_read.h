#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "aht20/aht20.h"
#include "bmp280/bmp280_read.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Ngưỡng thay đổi để quyết định gửi
#define TEMP_THRESHOLD      1.0f    // °C
#define HUMIDITY_THRESHOLD  5.0f    // %
#define PRESSURE_THRESHOLD  5.0f    // hPa
#define SEND_INTERVAL       pdMS_TO_TICKS(10000) // 10 giây

// Khai báo các task đọc riêng
void read_sensor_task(void *pvParameters);



#endif // SENSOR_READ_H
