#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_blufi_api.h"
#include "esp_blufi.h"
#include "time.h"
#include "driver/gpio.h"
/* Librerias componentes */
#include "ntp.h"
#include "mqtt.h"
#include "dht.h"
#include "soil.h"
#include "blufi.h"
#include "storage.h"
#include "bh1750.h" 
#include "pomp.h"
#include "header.h"

#define POWER_CTRL 4
#define ERASED     35 

static const char* TAG = "Button press";

SemaphoreHandle_t semaphoreWifiConection = NULL;
SemaphoreHandle_t semaphoreRTC = NULL;
SemaphoreHandle_t semaphoreLux = NULL;


TaskHandle_t xHandle = NULL;
TaskHandle_t xHandle2 = NULL;

sensor_data mediciones; 

config_data configuration;

void mqttServerConection(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(semaphoreWifiConection, portMAX_DELAY)) // establecida la conexión WiFi
        {
            adjust_time();
            mqtt_start();
        }
    }
}


void sensorsMeasurement(void *params)
{
    gpio_set_direction( POWER_CTRL, GPIO_MODE_OUTPUT );
    gpio_set_level(POWER_CTRL, 1);
    soilConfig();
    vTaskDelay(pdMS_TO_TICKS(2000));
    if(xSemaphoreTake(semaphoreRTC, portMAX_DELAY))
    {   
        xSemaphoreGive(semaphoreLux);
        while(true)
        {
            DHTerrorHandler(readDHT());
            humidity();
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}


void erased_nvs(void *params)
{
    gpio_config_t isr_config;
    isr_config.pin_bit_mask = (1ULL << ERASED);
    isr_config.mode = GPIO_MODE_INPUT;
    isr_config.pull_up_en = GPIO_PULLUP_ENABLE;
    isr_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    isr_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&isr_config);

    while(true)
    {
        int button_state = gpio_get_level(ERASED);
        if(button_state == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(3000));
            button_state = gpio_get_level(ERASED);
            if(button_state == 0)
            {
                ESP_LOGE(TAG, "Borrando NVS...");
                nvs_flash_erase();
                esp_restart();

            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void riego_auto(void * params)
{
    riego_config();
    while(true)
    {
        ESP_LOGI("RIEGO AUTO", "Entra a tarea de riego automático\n");

        if(mediciones.humedad_suelo < configuration.hum_inf)
        {
            int cant_riegos = 0;
            vTaskSuspend(xHandle2); //suspende tarea de medicion de humedad de suelo y DHT11
            while(mediciones.humedad_suelo < configuration.hum_sup && cant_riegos < 10)
            {
                regar();
                humidity();
                vTaskDelay(pdMS_TO_TICKS(20));
                cant_riegos += 1;
            }
            ultimo_riego();
            vTaskResume(xHandle2);
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}

void lux_sensor(void * params)
{
    if(xSemaphoreTake(semaphoreLux, portMAX_DELAY))
    {
        bh1750_init();

        while(true)
        {
            bh1750_read();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}

void app_main(void)
{
    semaphoreWifiConection = xSemaphoreCreateBinary();
    semaphoreRTC           = xSemaphoreCreateBinary();
    semaphoreLux           = xSemaphoreCreateBinary();

    blufi_start();

    if(NVS_read("UUID", configuration.UUID) == ESP_OK)
    {
        NVS_read("MAC", configuration.MAC);
        NVS_read("ultimo_riego", mediciones.ultimo_riego);

        nvs_handle_t my_handle;
        esp_err_t err = nvs_open("storage2", NVS_READWRITE, &my_handle);


        if(err != ESP_OK)
        {
            ESP_LOGI(TAG,"Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        }else{
            int result = 0; 
            if(nvs_get_i32(my_handle, "control_riego", &result) != ESP_OK)
            {
                configuration.control_riego = 0;
            }else{
                configuration.control_riego = result; 
            }
            if(nvs_get_i32(my_handle, "hum_sup", &result))
            {
                configuration.hum_sup = 60;
            }else{
                configuration.hum_sup = result; 
            }
            if(nvs_get_i32(my_handle, "hum_inf", &result))
            {
                configuration.hum_inf = 20; 
            }else{
                configuration.hum_inf = result;
            }
            nvs_close(my_handle);
        }

    }

    xTaskCreate(&mqttServerConection,
                "Conectando con HiveMQ Broker",
                4096,
                NULL,
                1,
                NULL);


    xTaskCreate(&sensorsMeasurement,
                "Comenzando mediciones de sensores",
                4096,
                NULL,
                1,
                &xHandle2);

    xTaskCreate(&erased_nvs,
                "Habilita borrado de NVS",
                2048,
                NULL,
                1,
                NULL);

    xTaskCreate(&riego_auto,
                "Inicia control de riego",
                2048,
                NULL,
                1,
                &xHandle);

    xTaskCreate(&lux_sensor,
                "Controla medicion de luz",
                2048,
                NULL,
                1,
                NULL);
                
    vTaskSuspend(xHandle);

}

