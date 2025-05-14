#ifndef MQTT_H
#define MQTT_H

#include "esp_err.h"

esp_err_t mqtt_init(void);
esp_err_t mqtt_publish(const char *topic, const char *data);
esp_err_t mqtt_publish_float(const char *topic, const char *key, float value);

#endif