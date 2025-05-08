#include "i2c_init.h"

static const char *TAG = "I2C_INIT";

esp_err_t i2c_master_init(i2c_port_t i2c_num, int sda_pin, int scl_pin, uint32_t clk_speed) {
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