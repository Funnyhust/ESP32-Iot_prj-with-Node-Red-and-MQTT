#include "wifi.h"
#include <stdint.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Định nghĩa thông tin Wi-Fi
static const char *TAG = "WIFI";
#define WIFI_SSID "ALL LUMI"
#define WIFI_PASS "lumivn274"

// Semaphore để báo hiệu Wi-Fi kết nối
static SemaphoreHandle_t wifi_connected_semaphore = NULL;

// Hàm lấy semaphore
SemaphoreHandle_t get_wifi_connected_semaphore(void) {
    return wifi_connected_semaphore;
}

// Hàm xử lý sự kiện Wi-Fi
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi bắt đầu, đang kết nối...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Mất kết nối Wi-Fi. Đang thử kết nối lại...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Nhận được IP: " IPSTR, IP2STR(&event->ip_info.ip));
        // Báo hiệu Wi-Fi đã kết nối
        if (wifi_connected_semaphore != NULL) {
            xSemaphoreGive(wifi_connected_semaphore);
        }
    }
}

// Hàm khởi động Wi-Fi
void wifi_init_sta(void) {
    ESP_LOGI(TAG, "Khởi động Wi-Fi...");

    // Tạo semaphore
    wifi_connected_semaphore = xSemaphoreCreateBinary();
    if (wifi_connected_semaphore == NULL) {
        ESP_LOGE(TAG, "Không tạo được semaphore");
        return;
    }

    // Khởi tạo NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Khởi tạo TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Cấu hình Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Đăng ký sự kiện Wi-Fi
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    // Thiết lập chế độ Station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Cấu hình SSID và Password
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Bắt đầu Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());
}