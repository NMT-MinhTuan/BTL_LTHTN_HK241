idf_component_register(SRCS "i2c_simple_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES wificon mqtttcp driver output i2c uart input)
