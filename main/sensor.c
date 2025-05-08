#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "bmp280/bmp280_read.h"
#include "aht20/aht20.h"
#include "i2c_init/i2c_init.h" // Bao gồm file header mới

static const char *TAG = "MAIN";

// Task đọc dữ liệu AHT20
static void aht20_read_task(void *pvParameters) {
    aht20_data_t data;

    while (1) {
        esp_err_t ret = aht20_read_data(&data);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "AHT20 - Nhiệt độ: %.2f °C, Độ ẩm: %.2f %% RH",
                     data.temperature, data.humidity);
        } else {
            ESP_LOGE(TAG, "Không đọc được dữ liệu từ AHT20");
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Chờ 2 giây
    }
}

void app_main(void) {
    // Khởi tạo I2C
    if (i2c_master_init(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ) != ESP_OK) {
        ESP_LOGE(TAG, "Khởi tạo I2C thất bại, dừng lại...");
        return;
    }

    // Khởi tạo AHT20
    if (aht20_init() != ESP_OK) {
        ESP_LOGE(TAG, "Khởi tạo AHT20 thất bại, dừng lại...");
        return;
    }

    // Khởi tạo BMP280
    if (bmp280_init() != ESP_OK) {
        ESP_LOGE(TAG, "Khởi tạo BMP280 thất bại, dừng lại...");
        return;
    }

    // Tạo task đọc AHT20
    BaseType_t task_created = xTaskCreate(aht20_read_task, "aht20_read_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Không tạo được task đọc AHT20");
        return;
    }

    // Tạo task đọc BMP280
    task_created = xTaskCreate(bmp280_read_task, "bmp280_read_task", 4096, NULL, 5, NULL);
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Không tạo được task đọc BMP280");
        return;
    }

    ESP_LOGI(TAG, "Task đọc AHT20 và BMP280 được tạo thành công");
}