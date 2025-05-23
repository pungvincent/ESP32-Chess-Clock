#ifndef LICHESS_H
#define LICHESS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "cJSON.h"

esp_err_t http_event_handler(esp_http_client_event_t *evt);
void fetch_lichess_game(void *pvParameters);

void activate_lichess_mode(void);
void deactivate_lichess_mode(void);

#endif // LICHESS_H