set(COMPONENT_REQUIRES esp_common bt log freertos esp_system esp_wifi esp_event nvs_flash esp_hw_support mbedtls esp_hw_support)
idf_component_register(SRCS "blufi.c" "blufi_security.c" "blufi_init.c"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES STORAGE HEADER
                        REQUIRES ${COMPONENT_REQUIRES})