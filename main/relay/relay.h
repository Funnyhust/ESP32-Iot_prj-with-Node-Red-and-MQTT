#ifndef RELAY_H
#define RELAY_H

#include <esp_err.h>

// Khởi tạo tất cả relay
esp_err_t relay_init(void);
// Bật hoặc tắt relay theo ID và trạng thái
// relay_id: ID của relay (1 hoặc 2)
esp_err_t relay(uint8_t relay_id, const char *state);

#endif // RELAY_H