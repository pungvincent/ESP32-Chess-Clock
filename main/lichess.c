
/** Put this in the src folder **/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "lichess.h"

//Set in menuconfig
#define LICHESS_TOKEN CONFIG_LICHESS_TOKEN

/** 
 * These lines declare external symbols for the start and end of an embedded 
 * PEM certificate file (lichess_cert.pem). 
 * 
 * The certificate is embedded at build time using `target_add_binary_data()` 
 * in the CMakeLists.txt file. ESP-IDF generates special symbols to access 
 * that binary data in memory:
 * 
 * - _binary_lichess_cert_pem_start: start of the certificate in memory
 * - _binary_lichess_cert_pem_end:   end of the certificate in memory
 * 
 * We declare these with `extern` and bind the C variable names to those 
 * linker-generated symbols using the `asm()` directive.
 */
extern const uint8_t lichess_cert_pem_start[] asm("_binary_lichess_cert_pem_start");
extern const uint8_t lichess_cert_pem_end[]   asm("_binary_lichess_cert_pem_end");

static const char *TAG_lichess = "Lichess"; 
TaskHandle_t lichessTaskHandle = NULL;

// HTTP event handler that runs on incoming data
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Print received data (JSON string) to console
            printf("Lichess response : %.*s\n", evt->data_len, (char *)evt->data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

// Task to periodically fetch the current Lichess game
void fetch_lichess_game(void *pvParameters) {
    while (1) {
        // Configure HTTPS request
        esp_http_client_config_t config = {
            .url = "https://lichess.org/api/account/playing",  // Lichess API endpoint
            .event_handler = http_event_handler,               // Callback for data reception
            .cert_pem = (const char *)lichess_cert_pem_start,  // TLS certificate
        };
        
        esp_http_client_handle_t client = esp_http_client_init(&config);  // Create client
        esp_http_client_set_method(client, HTTP_METHOD_GET);             // Use GET method

        // Set HTTP headers
        esp_http_client_set_header(client, "Authorization", LICHESS_TOKEN);  // Bearer token
        esp_http_client_set_header(client, "Accept", "application/json");    // Request JSON format

        // Perform the request
        esp_err_t err = esp_http_client_perform(client);

        // Log result
        if (err == ESP_OK) {
            ESP_LOGI(TAG_lichess, "Data sent.");
        } else {
            ESP_LOGE(TAG_lichess, "HTTP error %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);                // Free client resources
        vTaskDelay(pdMS_TO_TICKS(10000));               // Wait 10 seconds before repeating
    }
}


void activate_lichess_mode(void) {
    if (lichessTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            fetch_lichess_game,
            "fetch_lichess_game",
            8192,
            NULL,
            5,                  // priority > menu
            &lichessTaskHandle,
            1                   //core 1
        );
    }
}

void deactivate_lichess_mode(void) {
    if (lichessTaskHandle != NULL) {
        vTaskDelete(lichessTaskHandle);
        lichessTaskHandle = NULL;
    }
}