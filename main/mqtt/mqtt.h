#ifndef MQTT_H
#define MQTT_H

#include <esp_err.h>

// Các hàm public
esp_err_t mqtt_init(void);
esp_err_t mqtt_publish(const char *topic, const char *data);
esp_err_t mqtt_publish_float(const char *topic, const char *key, float value);
esp_err_t mqtt_subscribe(const char *topic, int qos);
int mqtt_handle_received_data(const char *topic, int topic_len, const char *data, int data_len);

// Khai báo hàm callback (chỉ prototype)
void mqtt_register_callback(void (*callback)(const char *, uint32_t, const char *, uint32_t));

#endif // MQTT_H
