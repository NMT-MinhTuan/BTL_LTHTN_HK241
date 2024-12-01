/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "wificon.h"
#include "mqtttcp.h"
#include "cJSON.h"

int value;
bool params;
const char *mqtt_topic;
const  char *mqtt_message;
float mqtt_data;
const char payload[20];
static const char *TAG = "MQTT_EXAMPLE";
static esp_mqtt_client_handle_t client ;
subcribe_callback_t subcribe_callback = NULL;
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        //printf("%.*s\r\n", event->data_len, event->data);
        char abc[40];
        snprintf(abc, sizeof(abc), "%.*s\r\n", event->data_len, event->data);
        printf("%s\n", abc);
        cJSON *json = cJSON_Parse(abc);
        if (json != NULL) {
            
            cJSON *method_item = cJSON_GetObjectItem(json, "method");
            if (method_item != NULL && cJSON_IsString(method_item)) {
                const char *method = method_item->valuestring;
                value = atoi(method); 
                ESP_LOGI("JSON", "Method: %d", value);
            }
            cJSON *params_item = cJSON_GetObjectItem(json, "params");
            if (params_item != NULL && cJSON_IsBool(params_item)) {
                params = cJSON_IsTrue(params_item);
                ESP_LOGI("JSON", "Params: %d", params);
            }
            cJSON_Delete(json);
            subcribe_callback(value,params);
        }

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            // log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            // log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            // log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}




static void mqtt_app_start(char* mqtt_uri, int mqtt_port, char* mqtt_username)
{
    esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = mqtt_uri,
    .broker.address.port = mqtt_port,  // Dùng port 1883 nếu không bảo mật, hoặc 8883 nếu bảo mật.
    .credentials.username = mqtt_username  // Thay "your_access_token" với access token của thiết bị
};
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}
void mqtt_send_task(char* mqtt_topic, char* mqtt_message) {
        int msg_id;
            
            printf("===========1==========\n");
            printf("%s\n",mqtt_topic);
            printf("%s\n",mqtt_message);
            printf("===========1==========\n");
            msg_id = esp_mqtt_client_publish(client, mqtt_topic, mqtt_message, 0, 1, 0);
            ESP_LOGI(TAG, "Sent message with ID: %d", msg_id);
    }

void connect_mqtt(char* mqtt_uri, int mqtt_port, char* mqtt_username)
{
    mqtt_app_start(mqtt_uri, mqtt_port, mqtt_username);
}
void subcribe_set_callback(void *cb){
    subcribe_callback = cb;
}