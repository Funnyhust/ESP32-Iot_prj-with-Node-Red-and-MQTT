#include "aht20/aht20.h"
#include "bmp280/bmp280_read.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>

#define TEMP_THRESHOLD      1.0    // °C
#define HUMIDITY_THRESHOLD  5.0    // %
#define PRESSURE_THRESHOLD  5.0    // hPa
#define SEND_INTERVAL       pdMS_TO_TICKS(10000) // 10 giây
//----------------------------
void read_aht20_task(void *pvParameters) {
    aht20_data_t data;
    float last_temp = -1000.0f;
    float last_humidity = -1000.0f;
    TickType_t last_temp_send_time = xTaskGetTickCount();
    TickType_t last_humid_send_time = xTaskGetTickCount();

    while (1) {
        if (aht20_read_data(&data) == ESP_OK) {
            float temp_diff = fabs(data.temperature - last_temp);
            float humid_diff = fabs(data.humidity - last_humidity);
            TickType_t now = xTaskGetTickCount();

            // Kiểm tra nhiệt độ
            if (temp_diff >= TEMP_THRESHOLD || (now - last_temp_send_time) >= SEND_INTERVAL) {
                ESP_LOGI("TEMP", "GỬI: Nhiệt độ %.2f°C", data.temperature);
                last_temp = data.temperature;
                last_temp_send_time = now;
                // TODO: Gửi MQTT hoặc xử lý thêm tại đây (nhiệt độ)
            }

            // Kiểm tra độ ẩm
            if (humid_diff >= HUMIDITY_THRESHOLD || (now - last_humid_send_time) >= SEND_INTERVAL) {
                ESP_LOGI("HUMIDITY", "GỬI: Độ ẩm %.2f%%", data.humidity);
                last_humidity = data.humidity;
                last_humid_send_time = now;
                // TODO: Gửi MQTT hoặc xử lý thêm tại đây (độ ẩm)
            }
        } else {
            ESP_LOGE("AHT20", "Không đọc được dữ liệu AHT20");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Đọc mỗi giây
    }
}


void read_pressure_task(void *pvParameters) {
    bmp280_data_t data;
    float last_pressure = -1000.0f;
    TickType_t last_send_time = xTaskGetTickCount();

    while (1) {
        if (bmp280_read_data(&data) == ESP_OK) {
            float press_diff = fabs(data.pressure - last_pressure);
            TickType_t now = xTaskGetTickCount();

            if (press_diff >= PRESSURE_THRESHOLD || (now - last_send_time) >= SEND_INTERVAL) {
                ESP_LOGI("PRESSURE", "GỬI: Áp suất %.2f hPa", data.pressure);
                last_pressure = data.pressure;
                last_send_time = now;
                // TODO: Gửi MQTT hoặc xử lý thêm tại đây
            }
        } else {
            ESP_LOGE("PRESSURE", "Không đọc được dữ liệu BMP280");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Đọc mỗi giây
    }
}
