#include "mqtt.h"
#include <esp_log.h>
#include <mqtt_client.h>
#include <cJSON.h>
#include <string.h>

static const char *TAG = "MQTT";
#define MQTT_BROKER_URI  "mqtt://192.168.89.137:1883"
#define MQTT_CLIENT_ID   "MQTT_ESP32_Client"
#define TOPIC_PUB        "sensor/data"
#define TOPIC_SUB        "control/relay"

static esp_mqtt_client_handle_t client = NULL;

// Callback user để báo dữ liệu về main.c
static void (*mqtt_user_callback)(const char *, uint32_t, const char *, uint32_t) = NULL;

void mqtt_register_callback(void (*callback)(const char *, uint32_t, const char *, uint32_t)) {
    mqtt_user_callback = callback;
}

int mqtt_handle_received_data(const char *topic, int topic_len, const char *data, int data_len) {
    // Đây chỉ là hàm parse đơn giản: trả về 1 hoặc 0 nếu topic "sensor/control"
    if (topic_len == strlen("control/relay") && strncmp(topic, "control/relay", topic_len) == 0) {
        if (data[0] == '1') {
            return 1;
        } else if (data[0] == '0') {
            return 0;
        }
    }
    return -1;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    ESP_LOGI(TAG, "MQTT event: %d", event->event_id);
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_subscribe(client, TOPIC_SUB, 1);
            ESP_LOGI(TAG, "Subscribed to %s", TOPIC_SUB);
            mqtt_publish_float(TOPIC_PUB, "temperature", 25.5);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT data received: topic=%.*s, data=%.*s", event->topic_len, event->topic, event->data_len, event->data);
            // Nếu callback đã đăng ký thì gọi nó
            if (mqtt_user_callback) {
                mqtt_user_callback(event->topic, event->topic_len, event->data, event->data_len);
            }
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT subscribed: %.*s", event->topic_len, event->topic);
            break;
        default:
            break;
    }
}

esp_err_t mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.client_id = MQTT_CLIENT_ID,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) {
        ESP_LOGE(TAG, "MQTT init failed");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MQTT start failed");
        return err;
    }
    return ESP_OK;
}

esp_err_t mqtt_publish(const char *topic, const char *data) {
    if (!client) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_FAIL;
    }
    int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Publish failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Published %s: %s", topic, data);
    return ESP_OK;
}

esp_err_t mqtt_publish_float(const char *topic, const char *key, float value) {
    if (!client) return ESP_FAIL;
    cJSON *root = cJSON_CreateObject();
    if (!root) return ESP_FAIL;
    cJSON_AddNumberToObject(root, key, value);
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json_str) return ESP_FAIL;

    esp_err_t err = mqtt_publish(topic, json_str);
    free(json_str);
    return err;
}

esp_err_t mqtt_subscribe(const char *topic, int qos) {
    if (!client) return ESP_FAIL;
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    if (msg_id < 0) return ESP_FAIL;
    ESP_LOGI(TAG, "Subscribed %s QoS %d", topic, qos);
    return ESP_OK;
}
