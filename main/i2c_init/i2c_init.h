#ifndef I2C_INIT_H
#define I2C_INIT_H

#include <driver/i2c.h>
#include <esp_log.h>

// Định nghĩa hằng số I2C
#define I2C_MASTER_SCL_IO           36      // GPIO pin cho SCL
#define I2C_MASTER_SDA_IO           37      // GPIO pin cho SDA
#define I2C_MASTER_NUM              I2C_NUM_0 // Cổng I2C
#define I2C_MASTER_FREQ_HZ          100000  // Tần số I2C

esp_err_t i2c_master_init(i2c_port_t i2c_num, int sda_pin, int scl_pin, uint32_t clk_speed);

#endif