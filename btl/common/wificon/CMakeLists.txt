idf_component_register(SRCS "wificon.c"
                           REQUIRES esp_wifi nvs_flash mqtttcp
                           INCLUDE_DIRS ".")