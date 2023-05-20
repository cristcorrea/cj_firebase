set(COMPONENT_REQUIRES freertos log esp_system esp_event esp_netif esp-tls esp_http_client)
idf_component_register(SRCS "http.c"
                        INCLUDE_DIRS "include"
                        REQUIRES ${COMPONENT_REQUIRES}
                        EMBED_TXTFILES certificate.pem
                        PRIV_REQUIRES HEADER CJSON)