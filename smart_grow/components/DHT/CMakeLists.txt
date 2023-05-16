set(COMPONENT_REQUIRES driver)
idf_component_register(SRCS "dht.c"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES HEADER
                        REQUIRES ${COMPONENT_REQUIRES})