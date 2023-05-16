set(COMPONENT_REQUIRES freertos esp_system nvs_flash mbedtls log)
idf_component_register(SRCS "storage.c"
                        INCLUDE_DIRS "include"
                        REQUIRES ${COMPONENT_REQUIRES})