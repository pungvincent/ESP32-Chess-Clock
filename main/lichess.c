
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

extern bool wifi_connected;

#define MAX_HTTP_OUTPUT_BUFFER 2048
static char output_buffer[MAX_HTTP_OUTPUT_BUFFER];
static int output_len = 0;

// HTTP event handler that runs on incoming data
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // This event is triggered when a chunk of HTTP response data is received.
            // If the server response is NOT chunked encoding, accumulate data into the buffer.
            if (!esp_http_client_is_chunked_response(evt->client)) {
                int copy_len = evt->data_len;
                // Ensure we do not overflow our buffer.
                if (output_len + copy_len < MAX_HTTP_OUTPUT_BUFFER) {
                    // Copy the incoming data chunk into the buffer, appending after previous data.
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                    output_len += copy_len;
                } else {
                    // Buffer overflow would occur, reset buffer to avoid corruption.
                    output_len = 0;
                }
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            // This event signals the end of the HTTP response.
            // Null-terminate the accumulated buffer so it forms a valid C string.
            output_buffer[output_len] = 0;

            // Parse the JSON data from the buffer.
            cJSON *json = cJSON_Parse(output_buffer);
            if (json == NULL) {
                // Parsing failed, likely invalid JSON. Log error and break.
                printf("JSON parsing error\n");
                break;
            }

            // Extract the "nowPlaying" array from the JSON root.
            cJSON *nowPlaying = cJSON_GetObjectItem(json, "nowPlaying");
            if (nowPlaying && cJSON_IsArray(nowPlaying)) {
                int array_size = cJSON_GetArraySize(nowPlaying);
                // Loop over each game object in the "nowPlaying" array.
                for (int i = 0; i < array_size; i++) {
                    cJSON *game = cJSON_GetArrayItem(nowPlaying, i);

                    // Extract the number of seconds left for this game.
                    cJSON *secondsLeft = cJSON_GetObjectItem(game, "secondsLeft");

                    // Extract the number of seconds left for this game.
                    cJSON *gameId = cJSON_GetObjectItem(game, "gameId");

                    // Extract the opponent object to get the username.
                    cJSON *opponent = cJSON_GetObjectItem(game, "opponent");
                    const char *username = NULL;
                    if (opponent) {
                        cJSON *username_json = cJSON_GetObjectItem(opponent, "username");
                        if (username_json && cJSON_IsString(username_json)) {
                            username = username_json->valuestring;
                        }
                    }

                    // Print opponent's username and time left for each game.
                    printf("Game %s: Opponent: %s, Seconds Left: %d\n",
                        gameId->valuestring,
                        username ? username : "Unknown",
                        secondsLeft ? secondsLeft->valueint : -1);
                }
            }

            // Free the parsed JSON object to avoid memory leaks.
            cJSON_Delete(json);

            // Reset the buffer length to zero, ready for the next HTTP response.
            output_len = 0;
            break;

        case HTTP_EVENT_DISCONNECTED:
            // The connection was lost or closed.
            // Clear the buffer length to avoid using stale data.
            output_len = 0;
            break;

        default:
            // Other HTTP events can be ignored for this handler.
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
        vTaskDelay(pdMS_TO_TICKS(3000));               // Wait 10 seconds before repeating
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
            0                   //core 1
        );
    }
}

void deactivate_lichess_mode(void) {
    if (lichessTaskHandle != NULL) {
        vTaskDelete(lichessTaskHandle);
        lichessTaskHandle = NULL;
    }
}