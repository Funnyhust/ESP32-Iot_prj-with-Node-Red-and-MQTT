#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

// Hàm khởi tạo và bắt đầu MQTT client
void mqtt_app_start(void);

// Hàm dùng để publish dữ liệu cảm biến
void mqtt_publish_sensor_data(const char *data);

// Đăng ký callback để nhận dữ liệu điều khiển từ Node-RED (ví dụ: bật/tắt cảm biến)
void mqtt_set_sensor_state_callback(void (*cb)(bool));

#endif // MQTT_CLIENT_H
