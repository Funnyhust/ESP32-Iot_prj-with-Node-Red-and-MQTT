// mqtt_client.c – Cấu hình và xử lý MQTT sử dụng ESP-IDF v5.4.1

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"          // Thư viện chính thức ESP-IDF MQTT
#include "my_mqtt_client.h"       // Header của bạn
#include <stddef.h>               // Cho NULL
#include <stdlib.h>               // malloc, free nếu cần

static const char *TAG = "MQTT_CLIENT";

static esp_mqtt_client_handle_t client = NULL;  // Biến toàn cục để lưu MQTT client

// Hàm callback xử lý các sự kiện từ MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Đã kết nối MQTT");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Mất kết nối MQTT");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Đã đăng ký topic thành công, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "Đã hủy đăng ký topic thành công, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Đã publish thành công, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Nhận dữ liệu:");
            printf("Topic: %.*s\r\n", event->topic_len, event->topic);
            printf("Dữ liệu: %.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Xảy ra lỗi MQTT");
            break;
        default:
            ESP_LOGW(TAG, "Sự kiện không xác định: %d", event->event_id);
            break;
    }
}

// Hàm khởi tạo và kết nối MQTT
void mqtt_app_start(void)
{
    // Cấu hình MQTT (thay đổi broker nếu cần)
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtt.eclipseprojects.io",  // Broker MQTT công cộng
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

// Hàm publish dữ liệu (ví dụ nhận chuỗi JSON từ cảm biến hoặc tương tự)
void mqtt_publish_sensor_data(const char *json_str)
{
    if (client == NULL) {
        ESP_LOGW(TAG, "Client chưa khởi tạo, không thể publish");
        return;
    }

    int msg_id = esp_mqtt_client_publish(client, "esp32/sensors", json_str, 0, 1, 0);
    ESP_LOGI(TAG, "Published: %s (msg_id=%d)", json_str, msg_id);
}
