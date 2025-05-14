#ifndef WIFI_H
#define WIFI_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

void wifi_init_sta(void);
SemaphoreHandle_t get_wifi_connected_semaphore(void);

#endif