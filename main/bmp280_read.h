#ifndef BMP280_READ_H
#define BMP280_READ_H

#include <freertos/FreeRTOS.h>
#include <esp_err.h>

// Cấu trúc lưu dữ liệu BMP280
typedef struct {
    float temperature; // Nhiệt độ (°C)
    float pressure;    // Áp suất (hPa)
    float altitude;    // Độ cao (m)
} bmp280_data_t;

// Hàm khởi tạo BMP280
esp_err_t bmp280_init(void);

// Hàm đọc dữ liệu từ BMP280
esp_err_t bmp280_read_data(bmp280_data_t *data);

// Hàm task đọc dữ liệu BMP280
void bmp280_read_task(void *pvParameters);

#endif // BMP280_READ_H