#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include "bmp280_read.h"
#include "aht20.h"

#define I2C_MASTER_SCL_IO           36      // GPIO pin cho SCL
#define I2C_MASTER_SDA_IO           37      // GPIO pin cho SDA
#define I2C_MASTER_NUM              I2C_NUM_0 // Cổng I2C
#define I2C_MASTER_FREQ_HZ          100000  // Tần số I2C

static const char *TAG = "MAIN";

// Hàm khởi tạo I2C
static esp_err_t i2c_master_init(i2c_port_t i2c_num, int sda_pin, int scl_pin, uint32_t clk_speed) {
    esp_err_t ret;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl_pin,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = clk_speed,
    };

    ret = i2c_param_config(i2c_num, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không cấu hình được I2C: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2c_driver_install(i2c_num, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không cài đặt được driver I2C: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "I2C khởi tạo thành công trên cổng %d", i2c_num);
    return ESP_OK;
}

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