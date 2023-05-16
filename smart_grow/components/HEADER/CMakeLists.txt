set(COMPONENT_REQUIRES )
idf_component_register(SRCS "header.c"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES RTC STORAGE
                        REQUIRES ${COMPONENT_REQUIRES})