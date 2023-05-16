set(COMPONENT_REQUIRES freertos soc log esp_adc )
idf_component_register(SRCS "soil.c"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES HEADER
                        REQUIRES ${COMPONENT_REQUIRES})