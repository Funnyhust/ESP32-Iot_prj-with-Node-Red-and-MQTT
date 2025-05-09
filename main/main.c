#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "bmp280/bmp280_read.h"
#include "aht20/aht20.h"
#include "i2c_init/i2c_init.h" // Bao gồm file header mới
#include "sensor_read/sensor_read.h" 

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
    BaseType_t task_created = xTaskCreate(read_aht20_task, "read_aht20_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Không tạo được task đọc nhiệt độ AHT20");
        return;
    }

    task_created = xTaskCreate(read_pressure_task, "read_pressure_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Không tạo được task đọc áp suất BMP280");
        return;
    }

}