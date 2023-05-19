set(COMPONENT_REQUIRES freertos log esp_system esp_event esp_netif esp_tls esp_http_client)
idf_component_register(SRCS "mqtt.c"
                        INCLUDE_DIRS "include"
                        REQUIRES ${COMPONENT_REQUIRES})