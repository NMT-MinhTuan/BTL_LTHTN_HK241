#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include "driver\gpio.h"
#include "hal/gpio_types.h"


void init_input(gpio_num_t gpio_num);
int get_level(gpio_num_t gpio);

#endif

