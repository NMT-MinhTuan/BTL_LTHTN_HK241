
#pragma once
//#include "D:\esp\Espressif\frameworks\esp-idf-v5.0.4\components\mqtt\esp-mqtt\include\mqtt_client.h"
#ifndef MQTTTCP_H
#define MQTTTCP_H


#include "mqtt_client.h"
#include "esp_log.h"

typedef void (*subcribe_callback_t) (int,bool);
void connect_mqtt(char* mqtt_uri, int mqtt_port, char* mqtt_username);
void mqtt_send_task(char* mqtt_topic, char* mqtt_message);
void subcribe_set_callback(void *cb);
#endif

