// my_mqtt_client.h – Header chứa khai báo hàm MQTT của bạn

#ifndef MY_MQTT_CLIENT_H
#define MY_MQTT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

// Hàm khởi tạo và kết nối MQTT
void mqtt_app_start(void);

// Hàm publish dữ liệu dạng chuỗi JSON
void mqtt_publish_sensor_data(const char *json_str);

#ifdef __cplusplus
}
#endif

#endif // MY_MQTT_CLIENT_H
