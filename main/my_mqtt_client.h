#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

// Khởi tạo và bắt đầu MQTT client
void mqtt_app_start(void);

// Hàm để gửi dữ liệu cảm biến (dạng JSON) lên MQTT broker
void mqtt_publish_sensor_data(float temp, float hum, float press);


#endif // MQTT_CLIENT_H
