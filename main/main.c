#include <stdio.h>
#include "my_mqtt_client.h"
#include "sensor_simulator.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

void app_main(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    nvs_flash_init();

    // TODO: Thêm Wi-Fi connect ở đây nếu muốn thực tế

    mqtt_app_start();

    while (1) {
        float temp = get_fake_temperature();
        float hum = get_fake_humidity();
        float press = get_fake_pressure();

        mqtt_publish_sensor_data(temp, hum, press);

        vTaskDelay(pdMS_TO_TICKS(5000));  // Gửi mỗi 5 giây
    }
}
