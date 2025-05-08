#include "bmp280_read.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <math.h>

#define I2C_MASTER_NUM              I2C_NUM_0 // Cổng I2C
#define BMP280_ADDR                 0x76    // Địa chỉ I2C của BMP280
#define SEA_LEVEL_PRESSURE_HPA      1013.25 // Áp suất mực nước biển tiêu chuẩn (hPa)

#define BMP280_REG_CTRL_MEAS        0xF4    // Thanh ghi điều khiển đo
#define BMP280_REG_CONFIG           0xF5    // Thanh ghi cấu hình
#define BMP280_REG_PRESS            0xF7    // Thanh ghi áp suất (3 byte)
#define BMP280_REG_TEMP             0xFA    // Thanh ghi nhiệt độ (3 byte)
#define BMP280_REG_CALIB            0x88    // Thanh ghi hiệu chỉnh (26 byte)

static const char *TAG = "BMP280";

// Biến lưu dữ liệu hiệu chỉnh
static struct {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
} calib_data;

// Khởi tạo BMP280
esp_err_t bmp280_init(void) {
    esp_err_t ret;
    i2c_cmd_handle_t cmd;

    // Cấu hình BMP280: chế độ bình thường, đo nhiệt độ và áp suất
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, BMP280_REG_CTRL_MEAS, true);
    i2c_master_write_byte(cmd, 0x25, true); // Nhiệt độ x1, áp suất x1, chế độ bình thường
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không cấu hình được BMP280: %s", esp_err_to_name(ret));
        return ret;
    }

    // Đọc dữ liệu hiệu chỉnh
    uint8_t calib_raw[26];
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, BMP280_REG_CALIB, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, calib_raw, sizeof(calib_raw), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không đọc được dữ liệu hiệu chỉnh: %s", esp_err_to_name(ret));
        return ret;
    }

    // Xử lý dữ liệu hiệu chỉnh
    calib_data.dig_T1 = (calib_raw[1] << 8) | calib_raw[0];
    calib_data.dig_T2 = (calib_raw[3] << 8) | calib_raw[2];
    calib_data.dig_T3 = (calib_raw[5] << 8) | calib_raw[4];
    calib_data.dig_P1 = (calib_raw[7] << 8) | calib_raw[6];
    calib_data.dig_P2 = (calib_raw[9] << 8) | calib_raw[8];
    calib_data.dig_P3 = (calib_raw[11] << 8) | calib_raw[10];
    calib_data.dig_P4 = (calib_raw[13] << 8) | calib_raw[12];
    calib_data.dig_P5 = (calib_raw[15] << 8) | calib_raw[14];
    calib_data.dig_P6 = (calib_raw[17] << 8) | calib_raw[16];
    calib_data.dig_P7 = (calib_raw[19] << 8) | calib_raw[18];
    calib_data.dig_P8 = (calib_raw[21] << 8) | calib_raw[20];
    calib_data.dig_P9 = (calib_raw[23] << 8) | calib_raw[22];

    ESP_LOGI(TAG, "BMP280 khởi tạo thành công");
    return ESP_OK;
}

// Xử lý dữ liệu thô BMP280
esp_err_t bmp280_process_data(uint8_t *raw_data, bmp280_data_t *data) {
    // Đọc dữ liệu nhiệt độ
    int32_t adc_T = (raw_data[3] << 12) | (raw_data[4] << 4) | (raw_data[5] >> 4);
    // Đọc dữ liệu áp suất
    int32_t adc_P = (raw_data[0] << 12) | (raw_data[1] << 4) | (raw_data[2] >> 4);

    // Tính toán nhiệt độ
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * ((int32_t)calib_data.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) * ((int32_t)calib_data.dig_T3)) >> 14;
    int32_t t_fine = var1 + var2;
    data->temperature = (float)((t_fine * 5 + 128) >> 8) / 100.0;

    // Tính toán áp suất
    int64_t var1_64, var2_64, p;
    var1_64 = ((int64_t)t_fine) - 128000;
    var2_64 = var1_64 * var1_64 * (int64_t)calib_data.dig_P6;
    var2_64 = var2_64 + ((var1_64 * (int64_t)calib_data.dig_P5) << 17);
    var2_64 = var2_64 + (((int64_t)calib_data.dig_P4) << 35);
    var1_64 = ((var1_64 * var1_64 * (int64_t)calib_data.dig_P3) >> 8) + ((var1_64 * (int64_t)calib_data.dig_P2) << 12);
    var1_64 = (((((int64_t)1) << 47) + var1_64)) * ((int64_t)calib_data.dig_P1) >> 33;
    if (var1_64 == 0) {
        return ESP_FAIL; // Tránh chia cho 0
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2_64) * 3125) / var1_64;
    var1_64 = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2_64 = (((int64_t)calib_data.dig_P8) * p) >> 19;
    p = ((p + var1_64 + var2_64) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
    data->pressure = (float)p / 25600.0; // Chuyển sang hPa

    // Tính độ cao
    data->altitude = 44330.0 * (1.0 - pow(data->pressure / SEA_LEVEL_PRESSURE_HPA, 0.1903));

    return ESP_OK;
}

// Đọc dữ liệu từ BMP280
esp_err_t bmp280_read_data(bmp280_data_t *data) {
    esp_err_t ret;
    uint8_t raw_data[6]; // 3 byte áp suất + 3 byte nhiệt độ

    // Đọc dữ liệu từ thanh ghi áp suất và nhiệt độ
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, BMP280_REG_PRESS, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, raw_data, sizeof(raw_data), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Không đọc được dữ liệu từ BMP280: %s", esp_err_to_name(ret));
        return ret;
    }

    // Xử lý dữ liệu
    ret = bmp280_process_data(raw_data, data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Lỗi khi xử lý dữ liệu BMP280: %s", esp_err_to_name(ret));
    }
    return ret;
}

// Task đọc dữ liệu BMP280
void bmp280_read_task(void *pvParameters) {
    bmp280_data_t data;

    while (1) {
        if (bmp280_read_data(&data) == ESP_OK) {
            ESP_LOGI(TAG, "Nhiệt độ: %.2f °C", data.temperature);
            ESP_LOGI(TAG, "Áp suất: %.2f hPa", data.pressure);
            ESP_LOGI(TAG, "Độ cao: %.2f m", data.altitude);
        } else {
            ESP_LOGE(TAG, "Không đọc được dữ liệu BMP280");
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Chờ 2 giây
    }
}