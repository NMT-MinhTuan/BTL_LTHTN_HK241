#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "i2c.h"
#include "esp_log.h"
#include "wificon.h"
#include "mqtttcp.h"
#include "output.h"
#include "uart.h"
#include "input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define WIFI_SSID      "TDLOGY"        // Thay bằng SSID của mạng Wi-Fi của bạn
#define WIFI_PASS      "lktd@12346" 
#define URI            "mqtt://mqtt.thingsboard.cloud"
#define PORT           1883
#define USERNAME       "RbomySKTs5mZAUoKwlYm"
#define TOPIC          "v1/devices/me/telemetry"
#define MESSAGE1       "tem"
#define MESSAGE2       "hum"
#define NUM_PHONE      "0778786442"
#define EN      GPIO_NUM_15


static const char *TAG = "esp32";
char cmd[100];

float t1,h1,t2,h2;

static void connect_wifi(char* ssid, char* password){
    tutorial_init();
    esp_err_t ret = tutorial_connect(ssid, password);
    if (ret == ESP_OK) {
        ESP_LOGE(TAG, "successful to connect to Wi-Fi network");
        
        }else{
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
        }
}

void send_mqtt(void *arg){
    while (1) {
        t2 = task_i2c('T');
        h2 = task_i2c('H');
        char data[30];
        snprintf(data, sizeof(data), "{\"%s\": %.2f}", "tem", t2);
        mqtt_send_task(TOPIC,data);
        snprintf(data, sizeof(data), "{\"%s\": %.2f}", "hum", h2);
        mqtt_send_task(TOPIC,data);
        vTaskDelay(pdMS_TO_TICKS(10000));  // Đo mỗi 2 giây
        
    }
}
void process_mess(char *c){
    if (strstr(c, "check")){
        t2 = task_i2c('T');
        h2 = task_i2c('H');
        sprintf(cmd, "temperature: %.2f *C\nhumidity: %.2f %%", t2,h2);
        send_sms("+84778786442", cmd);
    }
}
void sensor(void* arg){
    int a;
    while(1){
        a = get_level(14);
        if(a==1){
            printf("warning!!!\n");
            make_call("+84778786442");
            
            while(get_level(14) == 1);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void app_main() { 
    init(EN);
    set_level(EN,0);
    init_input(14);
    i2c_master_init();
    call_back_uart(&process_mess);
    module_sim();
    connect_wifi(WIFI_SSID,WIFI_PASS);
    connect_mqtt(URI, PORT, USERNAME);
    xTaskCreate(send_mqtt, "task_adc", 4096, NULL, 1, NULL);
    xTaskCreate(sensor, "sensor", 2048, NULL, 2, NULL);

}


  
