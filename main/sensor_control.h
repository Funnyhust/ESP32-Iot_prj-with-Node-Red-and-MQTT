/*#ifndef SENSOR_CONTROL_H
#define SENSOR_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Gửi dữ liệu cảm biến (temperature, humidity, pressure) dưới dạng JSON
void sensor_send_data(float temperature, float humidity, float pressure);

// Bật/tắt relay (relay_id: 1 hoặc 2; state: true = ON, false = OFF)
void sensor_control_set_relay_state(int relay_id, bool state);

// Khởi tạo các GPIO cho relay
void sensor_control_init(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_CONTROL_H
*/