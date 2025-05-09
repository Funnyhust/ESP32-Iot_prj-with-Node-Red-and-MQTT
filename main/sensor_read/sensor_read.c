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
void read_sensor_task(void *pvParameters) {
    aht20_data_t aht20_data;
    bmp280_data_t bmp280_data;

    float last_temp = -1000.0f;
    float last_humidity = -1000.0f;
    float last_pressure = -1000.0f;

    TickType_t last_temp_send_time = xTaskGetTickCount();
    TickType_t last_humid_send_time = xTaskGetTickCount();
    TickType_t last_pressure_send_time = xTaskGetTickCount();

    while (1) {
        TickType_t now = xTaskGetTickCount();

        // --- Đọc cảm biến AHT20 ---
        if (aht20_read_data(&aht20_data) == ESP_OK) {
            float temp_diff = fabs(aht20_data.temperature - last_temp);
            float humid_diff = fabs(aht20_data.humidity - last_humidity);

            // Kiểm tra nhiệt độ
            if (temp_diff >= TEMP_THRESHOLD || (now - last_temp_send_time) >= SEND_INTERVAL) {
                ESP_LOGI("TEMP", "GỬI: Nhiệt độ %.2f°C", aht20_data.temperature);
                last_temp = aht20_data.temperature;
                last_temp_send_time = now;
                // TODO: Gửi MQTT hoặc xử lý thêm tại đây (nhiệt độ)
            }

            // Kiểm tra độ ẩm
            if (humid_diff >= HUMIDITY_THRESHOLD || (now - last_humid_send_time) >= SEND_INTERVAL) {
                ESP_LOGI("HUMIDITY", "GỬI: Độ ẩm %.2f%%", aht20_data.humidity);
                last_humidity = aht20_data.humidity;
                last_humid_send_time = now;
                // TODO: Gửi MQTT hoặc xử lý thêm tại đây (độ ẩm)
            }
        } else {
            ESP_LOGE("AHT20", "Không đọc được dữ liệu AHT20");
        }

        // --- Đọc cảm biến BMP280 ---
        if (bmp280_read_data(&bmp280_data) == ESP_OK) {
            float press_diff = fabs(bmp280_data.pressure - last_pressure);

            if (press_diff >= PRESSURE_THRESHOLD || (now - last_pressure_send_time) >= SEND_INTERVAL) {
                ESP_LOGI("PRESSURE", "GỬI: Áp suất %.2f hPa", bmp280_data.pressure);
                last_pressure = bmp280_data.pressure;
                last_pressure_send_time = now;
                // TODO: Gửi MQTT hoặc xử lý thêm tại đây (áp suất)
            }
        } else {
            ESP_LOGE("BMP280", "Không đọc được dữ liệu BMP280");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Đọc mỗi giây
    }
}
