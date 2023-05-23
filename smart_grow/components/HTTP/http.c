#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "header.h"
#include "http_config.h"
#include "cJSON.h"


#include "esp_http_client.h"

static const char *TAG = "HTTP_CLIENT";

extern const uint8_t certificate_pem_start[] asm("_binary_certificate_pem_start");
extern const uint8_t certificate_pem_end[]   asm("_binary_certificate_pem_end");

extern sensor_data mediciones;
extern config_data configuration; 


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            //ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            ESP_LOGI(TAG, "%.*s\n", evt->data_len, (char*)evt->data);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}



void http_rest_with_url(void)
{

        int tamanio = strlen(FIRESTORE_URL)  + strlen(configuration.UUID) + strlen(configuration.MAC) + 2;
        char * request_url = malloc(tamanio); //
        ESP_LOGI(TAG, "Tamanio del url: %i\n", tamanio);
        int check = 1; 
        esp_http_client_handle_t client;
        if(request_url != NULL) 
        {
            memset(request_url, 0, tamanio);
            strcpy(request_url, FIRESTORE_URL); // url + coleccion
            strcat(request_url, configuration.UUID); // documento UUID
            //strcat(request_url, "/");
            //strcat(request_url, configuration.MAC); //coleccion 

            
            ESP_LOGI(TAG, "%s\n", request_url);

            esp_http_client_config_t config = 
            {
                .url = request_url,
                .event_handler = _http_event_handler,
                .cert_pem = (const  char*)certificate_pem_start,
            };
            
            client = esp_http_client_init(&config);
            check = 0; 
        }

    // POST
    if(check == 0)
    {


        cJSON * post_data = cJSON_CreateObject();
        cJSON * fields = cJSON_AddObjectToObject(post_data, "fields");
        cJSON_AddStringToObject(fields, "UUID", configuration.UUID);
        cJSON_AddStringToObject(fields, "MAC", configuration.MAC);
        cJSON_AddNumberToObject(fields, "hum_sup", configuration.hum_sup);
        cJSON_AddNumberToObject(fields, "hum_inf", configuration.hum_inf);
        cJSON_AddNumberToObject(fields, "control_riego", configuration.control_riego);
        char * post_data_str = cJSON_Print(post_data);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data_str, strlen(post_data_str));
        esp_http_client_set_header(client, "Content-Type", "application/json");

        ESP_LOGI(TAG, "%s\n", post_data_str);

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        }
        esp_http_client_cleanup(client);
        free(request_url);
        free(post_data_str);
        cJSON_Delete(post_data);
    }


/*
        cJSON * post_data = cJSON_CreateObject();
        cJSON * fields = cJSON_AddObjectToObject(post_data, "fields");
        cJSON * temp_amb = cJSON_AddObjectToObject(fields, "temp_amb");
        cJSON_AddNumberToObject(temp_amb, "doubleValue", mediciones.temperatura_amb);


*/



    // GET
    /*
    char local_response_buffer[350];
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    ESP_LOGI(TAG, "%s\n", local_response_buffer);
    //ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
    */

}

