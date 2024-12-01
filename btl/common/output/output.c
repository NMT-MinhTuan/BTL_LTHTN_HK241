#include <stdio.h>
#include <driver/gpio.h>
#include "output.h"

void init(gpio_num_t gpio_num){
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}
void set_level(gpio_num_t gpio_num,int level){
    gpio_set_level(gpio_num, level);
}

/*void init(gpio_num_t gpio_num)
{
    gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT_OUTPUT);
}

void set_level(gpio_num_t gpio_num, int level)
{  
    gpio_set_level(gpio_num, level);
}*/


