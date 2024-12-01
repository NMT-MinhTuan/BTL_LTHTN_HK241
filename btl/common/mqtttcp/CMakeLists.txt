idf_component_register(SRCS "mqtttcp.c"
                           REQUIRES esp_wifi nvs_flash wificon mqtt json
                           INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")