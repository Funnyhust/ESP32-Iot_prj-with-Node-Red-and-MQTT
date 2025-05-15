#include "relay.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <string.h>
#include <esp_err.h>
// Định nghĩa GPIO cho từng relay
static const gpio_num_t RELAY_GPIOS[] = {
    GPIO_NUM_47,  // Relay 1
    GPIO_NUM_21  // Relay 2
};
#define NUM_RELAYS 2
static const char *TAG = "RELAY";

// Khởi tạo tất cả relay
esp_err_t relay_init(void) {
    // Cấu hình GPIO cho tất cả relay
    uint64_t pin_bit_mask = 0;
    for (int i = 0; i < NUM_RELAYS; i++) {
        pin_bit_mask |= (1ULL << RELAY_GPIOS[i]);
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = pin_bit_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIOs: %d", err);
        return err;
    }

    // Tắt tất cả relay mặc định
    for (int i = 0; i < NUM_RELAYS; i++) {
        err = gpio_set_level(RELAY_GPIOS[i], 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set Relay %d GPIO %d level: %d", i + 1, RELAY_GPIOS[i], err);
            return err;
        }
    }

    ESP_LOGI(TAG, "Relays initialized");
    return ESP_OK;
}

esp_err_t relay(uint8_t relay_id, const char *state) {
    if (relay_id < 1 || relay_id > NUM_RELAYS) {
        ESP_LOGE(TAG, "Invalid relay ID: %d", relay_id);
        return ESP_ERR_INVALID_ARG;
    }

    gpio_num_t gpio = RELAY_GPIOS[relay_id - 1];
    int level;

    if (strcasecmp(state, "ON") == 0) {
        level = 1;
    } else if (strcasecmp(state, "OFF") == 0) {
        level = 0;
    } else {
        ESP_LOGE(TAG, "Invalid state: %s", state);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = gpio_set_level(gpio, level);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set Relay %d to %s", relay_id, state);
        return err;
    }

    ESP_LOGI(TAG, "Relay %d turned %s", relay_id, level ? "ON" : "OFF");
    return ESP_OK;
}
