#ifndef OUTPUT_H
#define OUTPUT_H
#include "esp_err.h"
#include "hal/gpio_types.h"

void init(gpio_num_t gpio_num);
void set_level(gpio_num_t gpio_num,int level);

#endif