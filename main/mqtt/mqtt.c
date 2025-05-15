#include "mqtt.h"
#include <esp_log.h>
#include <mqtt_client.h>
#include <cJSON.h>
#include <esp_event.h>
#include <string.h>

// Định nghĩa thông tin MQTT
static const char *TAG = "MQTT";
#define MQTT_BROKER_URI  "mqtt://192.168.89.137:1883"
#define MQTT_CLIENT_ID "MQTT_ESP32_Client"
#define TOPIC_PUB "sensor/data"

static esp_mqtt_client_handle_t client;

// Hàm xử lý sự kiện MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected to broker");
            // Gửi dữ liệu JSON mẫu khi kết nối thành công
            mqtt_publish_float(TOPIC_PUB, "temperature", 25.5);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected from broker");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT published, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error: errno=%d", event->error_handle->esp_transport_sock_errno);
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGE(TAG, "TLS error: %d", event->error_handle->esp_tls_last_esp_err);
            }
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "Bắt đầu kết nối tới broker...");
            break;
        default:
            ESP_LOGI(TAG, "Other MQTT event id: %d", event->event_id);
            break;
    }
}

// Khởi tạo và kết nối tới MQTT broker
esp_err_t mqtt_init(void) {
    ESP_LOGI(TAG, "Khởi động MQTT với broker: %s", MQTT_BROKER_URI);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.client_id = MQTT_CLIENT_ID,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) {
        ESP_LOGE(TAG, "Không khởi tạo được MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Không khởi động được MQTT client: %d", err);
        return err;
    }

    return ESP_OK;
}

// Gửi dữ liệu lên MQTT broker (chuỗi)
esp_err_t mqtt_publish(const char *topic, const char *data) {
    if (!client) {
        ESP_LOGE(TAG, "MQTT client chưa được khởi tạo");
        return ESP_FAIL;
    }

    int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Không gửi được dữ liệu tới topic %s", topic);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Đã gửi tới %s: %s", topic, data);
    return ESP_OK;
}

// Gửi dữ liệu cảm biến dạng số thực (chuyển thành JSON với key tùy chỉnh)
esp_err_t mqtt_publish_float(const char *topic, const char *key, float value) {
    if (!client) {
        ESP_LOGE(TAG, "MQTT client chưa được khởi tạo");
        return ESP_FAIL;
    }

    if (!key || key[0] == '\0') {
        ESP_LOGE(TAG, "Key JSON không hợp lệ");
        return ESP_FAIL;
    }

    // Tạo đối tượng JSON
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        ESP_LOGE(TAG, "Không tạo được đối tượng JSON");
        return ESP_FAIL;
    }

    // Thêm giá trị float vào JSON với key tùy chỉnh
    cJSON_AddNumberToObject(root, key, value);

    // Chuyển JSON thành chuỗi
    char *json_string = cJSON_PrintUnformatted(root);
    if (!json_string) {
        ESP_LOGE(TAG, "Không chuyển được JSON thành chuỗi");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Gửi chuỗi JSON qua MQTT
    esp_err_t err = mqtt_publish(topic, json_string);

    // Giải phóng bộ nhớ
    cJSON_Delete(root);
    free(json_string);

    return err;
}