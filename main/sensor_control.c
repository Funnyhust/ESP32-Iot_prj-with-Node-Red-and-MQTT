/*#include "sensor_control.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define RELAY1_GPIO 18
#define RELAY2_GPIO 19

static const char *TAG = "SENSOR_CTRL";

// Hàm khởi tạo GPIO cho các relay
void sensor_control_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY1_GPIO) | (1ULL << RELAY2_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Tắt relay ban đầu
    gpio_set_level(RELAY1_GPIO, 0);
    gpio_set_level(RELAY2_GPIO, 0);
    ESP_LOGI(TAG, "Relay GPIOs initialized");
}

// Gửi dữ liệu cảm biến dưới dạng JSON
void sensor_send_data(float temperature, float humidity, float pressure) {
    char json[128];

    // Tạo chuỗi JSON
    snprintf(json, sizeof(json),
             "{\"temperature\": %.2f, \"humidity\": %.2f, \"pressure\": %.2f}",
             temperature, humidity, pressure);

    // Gửi lên MQTT
    mqtt_publish_sensor_data(json);
    ESP_LOGI(TAG, "Published sensor data: %s", json);
}

// Điều khiển relay (1 hoặc 2)
void sensor_control_set_relay_state(int relay_id, bool state) {
    int gpio = (relay_id == 1) ? RELAY1_GPIO : RELAY2_GPIO;
    gpio_set_level(gpio, state ? 1 : 0);
    ESP_LOGI(TAG, "Relay %d turned %s", relay_id, state ? "ON" : "OFF");
}
*/