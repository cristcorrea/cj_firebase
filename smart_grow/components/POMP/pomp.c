#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "pomp.h"


#define BOMBA 18

void riego_config()
{
    gpio_config_t riego_config;
    riego_config.pin_bit_mask = (1ULL << BOMBA);
    riego_config.mode = GPIO_MODE_OUTPUT;
    riego_config.pull_up_en = GPIO_PULLUP_DISABLE;
    riego_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    riego_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&riego_config);

}

void regar()
{
    gpio_set_level(BOMBA, 1);
    vTaskDelay(5000/portTICK_PERIOD_MS);
    gpio_set_level(BOMBA, 0);
}
