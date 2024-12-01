#include "input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


gpio_config_t myINPUTconfig;

void init_input(gpio_num_t gpio_num){
    myINPUTconfig.pin_bit_mask = (1ULL << gpio_num);
    myINPUTconfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    myINPUTconfig.pull_up_en = GPIO_PULLUP_ENABLE;
    myINPUTconfig.mode = GPIO_MODE_INPUT;
    gpio_config(&myINPUTconfig);
}

int get_level(gpio_num_t gpio){
    return gpio_get_level(gpio);
}
