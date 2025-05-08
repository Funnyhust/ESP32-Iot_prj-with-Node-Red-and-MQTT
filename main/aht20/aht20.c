#include "aht20.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/i2c.h>

static const char *TAG = "AHT20";

// Kiểm tra trạng thái AHT20
static esp_err_t aht20_check_status(void) {
    uint8_t status;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ATH20_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &status, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không đọc được trạng thái: %s", esp_err_to_name(ret));
        return ret;
    }
    return (status & 0x80) ? ESP_FAIL : ESP_OK;
}

// Khởi tạo cảm biến AHT20
esp_err_t aht20_init(void) {
    esp_err_t ret;
    i2c_cmd_handle_t cmd;

    // Gửi lệnh khởi tạo (0xBE 0x08 0x00)
    uint8_t init_cmd[] = {ATH20_CMD_INIT, 0x08, 0x00};
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ATH20_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, init_cmd, sizeof(init_cmd), true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không khởi tạo được AHT20: %s", esp_err_to_name(ret));
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Chờ khởi tạo

    // Kiểm tra trạng thái
    ret = aht20_check_status();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT20 không sẵn sàng");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "AHT20 khởi tạo thành công");
    return ESP_OK;
}

// Đọc dữ liệu từ cảm biến AHT20
esp_err_t aht20_read_data(aht20_data_t *data) {
    esp_err_t ret;
    uint8_t raw_data[6];

    // Gửi lệnh đo (0xAC 0x33 0x00)
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ATH20_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, ATH20_CMD_TRIGGER_MEASUREMENT, true);
    i2c_master_write_byte(cmd, 0x33, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không gửi được lệnh đo: %s", esp_err_to_name(ret));
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(80)); // Đợi cảm biến đo xong

    // Đọc dữ liệu trả về từ cảm biến
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ATH20_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, raw_data, sizeof(raw_data), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không đọc được dữ liệu từ AHT20: %s", esp_err_to_name(ret));
        return ret;
    }

    return aht20_process_data(raw_data, data);
}

// Xử lý dữ liệu đọc được từ cảm biến
esp_err_t aht20_process_data(uint8_t *raw_data, aht20_data_t *data) {
    // Debug raw data
    ESP_LOGI(TAG, "Raw: %02X %02X %02X %02X %02X %02X",
             raw_data[0], raw_data[1], raw_data[2],
             raw_data[3], raw_data[4], raw_data[5]);

    // Kiểm tra bit busy (bit 7 của byte 0)
    if (raw_data[0] & 0x80) {
        ESP_LOGW(TAG, "Cảm biến đang bận, thử lại sau");
        return ESP_FAIL;
    }

    // Tách dữ liệu
    uint32_t raw_humidity = ((uint32_t)raw_data[1] << 12) |
                            ((uint32_t)raw_data[2] << 4) |
                            ((raw_data[3] & 0xF0) >> 4);

    uint32_t raw_temp = ((uint32_t)(raw_data[3] & 0x0F) << 16) |
                        ((uint32_t)raw_data[4] << 8) |
                        raw_data[5];

    // Tính toán nhiệt độ và độ ẩm
    data->humidity = (float)raw_humidity * 100.0 / 1048576.0; // 2^20 = 1048576
    data->temperature = (float)raw_temp * 200.0 / 1048576.0 - 50.0;

    ESP_LOGI(TAG, "Nhiệt độ: %.2f °C, Độ ẩm: %.2f %% RH",
             data->temperature, data->humidity);

    return ESP_OK;
}