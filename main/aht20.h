#ifndef AHT20_H
#define AHT20_H

#include <freertos/FreeRTOS.h>
#include <esp_err.h>

#define I2C_MASTER_NUM              I2C_NUM_0 // Cổng I2C
#define ATH20_ADDR                  0x38      // Địa chỉ I2C của AHT20
#define ATH20_CMD_INIT              0xBE      // Lệnh khởi tạo
#define ATH20_CMD_TRIGGER_MEASUREMENT 0xAC   // Lệnh kích hoạt đo

// Cấu trúc lưu dữ liệu AHT20
typedef struct {
    float temperature; // Nhiệt độ (°C)
    float humidity;    // Độ ẩm (% RH)
} aht20_data_t;

// Hàm khởi tạo AHT20
esp_err_t aht20_init(void);

// Hàm đọc dữ liệu từ AHT20
esp_err_t aht20_read_data(aht20_data_t *data);

// Hàm xử lý dữ liệu AHT20
esp_err_t aht20_process_data(uint8_t *raw_data, aht20_data_t *data);

#endif // AHT20_H