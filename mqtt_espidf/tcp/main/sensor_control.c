#include "sensor_control.h"
#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "SENSOR";
static bool sensor_enabled = false; // Trạng thái cảm biến: ON/OFF

// Khởi tạo cảm biến và đăng ký callback để nhận điều khiển từ MQTT
void sensor_init(void) {
    mqtt_set_sensor_state_callback(sensor_set_enabled);
}

// Bật hoặc tắt cảm biến theo yêu cầu từ MQTT
void sensor_set_enabled(bool enabled) {
    sensor_enabled = enabled;
    ESP_LOGI(TAG, "Sensor turned %s", enabled ? "ON" : "OFF");
}

// Nếu cảm biến đang bật, gửi giá trị mock (giả lập) lên MQTT
void sensor_send_mock_data(void) {
    if (sensor_enabled) {
        const char *mock_temp = "25.3"; // Giá trị giả lập nhiệt độ
        mqtt_publish_sensor_data(mock_temp);
    }
}
