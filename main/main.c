#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "bmp280/bmp280_read.h"
#include "aht20/aht20.h"
#include "i2c_init/i2c_init.h" // Bao gồm file header mới
#include "sensor_read/sensor_read.h" 
#include "wifi/wifi.h" 
#include "mqtt/mqtt.h"

static const char *TAG = "MAIN";

// Task đọc dữ liệu AHT20

void app_main(void) {
    esp_err_t ret;

    // Khởi tạo I2C với cổng I2C_NUM_0, chân SDA = 37, SCL = 36 và tốc độ 100 kHz
    ret = i2c_master_init(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
    if (ret == ESP_OK) {
        ESP_LOGI("APP", "I2C initialization successful");
    } else {
        ESP_LOGE("APP", "I2C initialization failed with error code %s", esp_err_to_name(ret));
    }
    bmp280_init();
    // Khởi động Wi-Fi
wifi_init_sta();

// Lấy semaphore Wi-Fi
SemaphoreHandle_t wifi_semaphore = get_wifi_connected_semaphore();
if (wifi_semaphore == NULL) {
    ESP_LOGE(TAG, "Không lấy được semaphore Wi-Fi");
    return;
}

// Chờ Wi-Fi kết nối
ESP_LOGI(TAG, "Đang chờ Wi-Fi kết nối...");
if (xSemaphoreTake(wifi_semaphore, portMAX_DELAY) == pdTRUE) {
    ESP_LOGI(TAG, "Wi-Fi đã kết nối, khởi động MQTT...");
    // Khởi động MQTT
    ret = mqtt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MQTT initialization failed with error code %s", esp_err_to_name(ret));
        return;
    }

    // Tạo task đọc cảm biến
    BaseType_t task_created = xTaskCreate(read_sensor_task, "read_sensor_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Không tạo được task đọc cảm biến");
        return;
    }
} else {
    ESP_LOGE(TAG, "Không thể chờ Wi-Fi kết nối");
}
//   BaseType_t  task_created = xTaskCreate(bmp280_read_task, "read_aht20_task", 4096, NULL, 5, NULL);
//     if (task_created != pdPASS) {
//         ESP_LOGE(TAG, "Không tạo được task đọc nhiệt độ AHT20");
//         return;
//     }
}