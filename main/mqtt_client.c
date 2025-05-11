#include <stdio.h>
#include <stddef.h>
#include "mqtt_client.h"
#include "my_mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "MQTT";  // Tag dùng để in log
esp_mqtt_client_handle_t client = NULL;  // Handle MQTT client toàn cục

// Hàm callback xử lý các sự kiện MQTT
static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t esp_event_base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // Khi kết nối thành công, có thể đăng ký topic nhận điều khiển
            esp_mqtt_client_subscribe(client, "esp32/control", 1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");  // Ngắt kết nối khỏi broker
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);  // Đăng ký topic thành công
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id); // Hủy đăng ký topic
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);  // Gửi dữ liệu thành công
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");  // Khi nhận dữ liệu từ broker
            printf("Topic: %.*s\r\n", event->topic_len, event->topic);
            printf("Data: %.*s\r\n", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");  // Có lỗi xảy ra trong quá trình kết nối
            break;

        default:
            ESP_LOGW(TAG, "Other event id:%d", event->event_id);  // Các sự kiện khác chưa xử lý
            break;
    }
}

// Hàm khởi tạo MQTT client và bắt đầu kết nối đến broker
void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",  // Lấy URL từ menuconfig
    };

    client = esp_mqtt_client_init(&mqtt_cfg);  // Khởi tạo client với config
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);  // Đăng ký callback
    esp_mqtt_client_start(client);  // Bắt đầu kết nối
}

// Hàm publish dữ liệu sensor lên broker dưới dạng JSON
void mqtt_publish_sensor_data(float temp, float hum, float press) {
    char json_str[128];

    // Chuyển các giá trị sensor thành chuỗi JSON
    snprintf(json_str, sizeof(json_str),
             "{\"temp\": %.2f, \"humidity\": %.2f, \"pressure\": %.2f}",
             temp, hum, press);

    // Gửi chuỗi JSON lên topic "esp32/sensors"
    esp_mqtt_client_publish(client, "esp32/sensors", json_str, 0, 1, 0);
}
