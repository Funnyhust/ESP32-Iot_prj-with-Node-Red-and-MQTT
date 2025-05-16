#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include "button/button.h"
#include "relay/relay.h"
#include "bmp280/bmp280_read.h"
#include "aht20/aht20.h"
#include "i2c_init/i2c_init.h"
#include "sensor_read/sensor_read.h"
#include "wifi/wifi.h"
#include "mqtt/mqtt.h"

static const char *TAG = "MAIN";
static bool relay1_status = false;

void button_callback(btn_event_e event, uint8_t button_id) {
    if (event == PRESSED) {
        relay1_status = !relay1_status;
        if (relay1_status) {
            relay(1, "ON");
            mqtt_publish_float("relay/control", "relay", 1);
            ESP_LOGI(TAG, "Button %d pressed: Relay 1 ON", button_id);
        } else {
            relay(1, "OFF");
            mqtt_publish_float("relay/control", "relay", 0);
            ESP_LOGI(TAG, "Button %d pressed: Relay 1 OFF", button_id);
        }
    }
}

// Callback nhận dữ liệu MQTT từ mqtt.c
void mqtt_callback(const char *topic, uint32_t topic_len, const char *data, uint32_t data_len) {
    int cmd = mqtt_handle_received_data(topic, topic_len, data, data_len);
    if (cmd == 1) {
        relay(1, "ON");
        relay1_status = true;
        ESP_LOGI(TAG, "MQTT command: Relay 1 ON");
    } else if (cmd == 0) {
        relay(1, "OFF");
        relay1_status = false;
        ESP_LOGI(TAG, "MQTT command: Relay 1 OFF");
    } else {
        ESP_LOGW(TAG, "Invalid MQTT command or topic");
    }
}

void app_main(void) {
    esp_err_t ret;

    // Khởi tạo button
    ESP_LOGI(TAG, "Khởi tạo button...");
    ret = button_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Button initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    // Đăng ký nút nhấn
    uint8_t button1_id;
    if (button_add(GPIO_NUM_16, button_callback, &button1_id) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add button");
        return;
    }

    // Khởi tạo relay
    ESP_LOGI(TAG, "Khởi tạo relay...");
    ret = relay_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Relay initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    // Khởi tạo I2C
    ESP_LOGI(TAG, "Khởi tạo I2C...");
    ret = i2c_master_init(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    // Khởi tạo BMP280
    ESP_LOGI(TAG, "Khởi tạo BMP280...");
    ret = bmp280_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BMP280 initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    // Khởi tạo AHT20 (giả sử cần cho sensor_read_task)
    ESP_LOGI(TAG, "Khởi tạo AHT20...");
    ret = aht20_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT20 initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    // Khởi động Wi-Fi
    ESP_LOGI(TAG, "Khởi động Wi-Fi...");
    wifi_init_sta();

    // Chờ Wi-Fi kết nối
    ESP_LOGI(TAG, "Đang chờ Wi-Fi kết nối...");
    SemaphoreHandle_t wifi_semaphore = get_wifi_connected_semaphore();
    if (wifi_semaphore == NULL) {
        ESP_LOGE(TAG, "Không lấy được semaphore Wi-Fi");
        return;
    }
    if (xSemaphoreTake(wifi_semaphore, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Không thể chờ Wi-Fi kết nối");
        return;
    }
    ESP_LOGI(TAG, "Wi-Fi đã kết nối");

    // Khởi động MQTT
    ESP_LOGI(TAG, "Khởi động MQTT...");
    ret = mqtt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MQTT initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    mqtt_register_callback(mqtt_callback);
    ESP_LOGI(TAG, "MQTT khởi động thành công");

    // Tạo task đọc cảm biến
    ESP_LOGI(TAG, "Tạo task đọc cảm biến...");
    BaseType_t task_created = xTaskCreate(read_sensor_task, "read_sensor_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Không tạo được task đọc cảm biến");
        return;
    }
    ESP_LOGI(TAG, "Task đọc cảm biến được tạo thành công");
}
