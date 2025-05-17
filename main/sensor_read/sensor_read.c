#include "aht20/aht20.h"
#include "bmp280/bmp280_read.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <math.h>
#include "mqtt/mqtt.h"
#include "relay/relay.h"
#include "sensor_read.h"

#define TAG                 "SENSOR"
#define TEMP_THRESHOLD      1.0f    // °C
#define HUMIDITY_THRESHOLD  5.0f    // %
#define PRESSURE_THRESHOLD  5.0f    // hPa
#define SEND_INTERVAL       pdMS_TO_TICKS(10000) // 10 giây
#define TOPIC_PUB           "sensor/data" // Topic MQTT để gửi dữ liệu
#define MAX_TEMP            31.0   // Nhiệt độ tối đa (°C)
#define MIN_TEMP            30.0   // Nhiệt độ tối thiểu (°C)

// Định nghĩa biến toàn cục (không dùng static)
sensor_data_t sensor_data = {0};
SemaphoreHandle_t sensor_data_mutex;

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

        // Đọc cảm biến AHT20
        if (aht20_read_data(&aht20_data) == ESP_OK) {
            float temp_diff = fabs(aht20_data.temperature - last_temp);
            float humid_diff = fabs(aht20_data.humidity - last_humidity);

            // Cập nhật dữ liệu toàn cục
            xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
            sensor_data.temperature = aht20_data.temperature;
            sensor_data.humidity = aht20_data.humidity;
            xSemaphoreGive(sensor_data_mutex);

            // Kiểm tra và gửi nhiệt độ
            if (temp_diff >= TEMP_THRESHOLD || (now - last_temp_send_time) >= SEND_INTERVAL) {
                mqtt_publish_float(TOPIC_PUB, "temperature", aht20_data.temperature);
                ESP_LOGI(TAG, "GỬI: Nhiệt độ %.2f°C", aht20_data.temperature);
                last_temp = aht20_data.temperature;
                last_temp_send_time = now;
            }

            // Kiểm tra và gửi độ ẩm
            if (humid_diff >= HUMIDITY_THRESHOLD || (now - last_humid_send_time) >= SEND_INTERVAL) {
                mqtt_publish_float(TOPIC_PUB, "humidity", aht20_data.humidity);
                ESP_LOGI(TAG, "GỬI: Độ ẩm %.2f%%", aht20_data.humidity);
                last_humidity = aht20_data.humidity;
                last_humid_send_time = now;
            }
        } else {
            ESP_LOGE(TAG, "Không đọc được dữ liệu AHT20");
        }

        // Đọc cảm biến BMP280
        if (bmp280_read_data(&bmp280_data) == ESP_OK) {
            float press_diff = fabs(bmp280_data.pressure - last_pressure);

            // Cập nhật dữ liệu toàn cục
            xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
            sensor_data.pressure = bmp280_data.pressure;
            xSemaphoreGive(sensor_data_mutex);

            // Kiểm tra và gửi áp suất
            if (press_diff >= PRESSURE_THRESHOLD || (now - last_pressure_send_time) >= SEND_INTERVAL) {
                mqtt_publish_float(TOPIC_PUB, "pressure", bmp280_data.pressure);
                ESP_LOGI(TAG, "GỬI: Áp suất %.2f hPa", bmp280_data.pressure);
                last_pressure = bmp280_data.pressure;
                last_pressure_send_time = now;
            }
        } else {
            ESP_LOGE(TAG, "Không đọc được dữ liệu BMP280");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Đọc mỗi giây
    }
}

void warning_task(void *pvParameters) {
    bool last_relay_status = false;

    while (1) {
        float temperature;

        // Lấy dữ liệu an toàn
        xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
        temperature = sensor_data.temperature;
        xSemaphoreGive(sensor_data_mutex);

        ESP_LOGI(TAG, "Nhiệt độ: %.2f°C", temperature);

        // Kiểm tra nhiệt độ và điều khiển relay
        if (temperature >= MAX_TEMP && !last_relay_status) {
            ESP_LOGW(TAG, "Nhiệt độ cao! Tắt relay");
            relay(2, "ON");
            last_relay_status = true;
        } else if (temperature <= MIN_TEMP && last_relay_status) {
            ESP_LOGI(TAG, "Nhiệt độ thấp. Bật relay");
            relay(2, "OFF");
            last_relay_status = false;
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Kiểm tra mỗi 5 giây
    }
}