#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "uart.h"
#define SIM_UART_NUM UART_NUM_1     // Sử dụng UART1 cho module SIM
#define UART_SIM_TX_PIN (17)   // TX của ESP32 nối với RX của module SIM
#define UART_SIM_RX_PIN (16)   // RX của ESP32 nối với TX của module SIM
#define UART_SIM_BAUD_RATE (115200) // Baud rate của SIM A7680C

#define BUF_SIZE (1024)

static const char *TAG = "SIM_A7680C";
func_ptr isr1 = NULL;


// Gửi lệnh AT đến module SIM
void send_at_command(const char *command) {
    uart_write_bytes(SIM_UART_NUM, command, strlen(command));
    uart_write_bytes(SIM_UART_NUM, "\r\n", 2);
    ESP_LOGI(TAG, "Sent command: %s", command);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Chờ module xử lý
}
void make_call(const char *phone_number) {
    char command[32];
    snprintf(command, sizeof(command), "ATD%s;", phone_number);
    ESP_LOGI(TAG, "Sent command: %s", command);
    uart_write_bytes(SIM_UART_NUM, command, strlen(command));
    uart_write_bytes(SIM_UART_NUM, "\r\n", 2);
    //ESP_LOGI(TAG, "Sent command: %s", command);
    vTaskDelay(pdMS_TO_TICKS(500));
}
void send_sms(const char *phone_number, const char *message) {
    send_at_command("AT+CMGF=1");
    // Thiết lập số điện thoại người nhận
    char cmd[50];
    snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"", phone_number);
    send_at_command(cmd);

    // Gửi nội dung tin nhắn
    uart_write_bytes(SIM_UART_NUM, message, strlen(message));
    uart_write_bytes(SIM_UART_NUM, "\x1A", 1); // Ký tự Ctrl+Z để kết thúc tin nhắn

    ESP_LOGI(TAG, "SMS sent to %s: %s", phone_number, message);
}
// Xử lý phản hồi từ module SIM
void process_sms_response(const char *response) {
    ESP_LOGI(TAG, "SMS Response: %s", response);

    // Kiểm tra nếu là tin nhắn SMS
    if (strstr(response, "+CMT:")) {
        char phone_number[20] ;
        char message[50] ;

        // Trích xuất số điện thoại và nội dung tin nhắn
        
        sscanf(response, "\n+CMT: \"%[^\"]\",%*s\n%[^\n]", phone_number, message);
        //sscanf( response, "\n+CMT: %14s,"",%*s\n%1s", phone_number, message);
        // Kiểm tra kết quả
        //if (ret == 2) {
        
        ESP_LOGI(TAG, "Received SMS from: %s", phone_number);
        ESP_LOGI(TAG, "Message: %s", message);
        
        //} else {
            //printf("Failed to parse SMS.\n");
        send_at_command("AT+CMGD=1,4");
        isr1(message);
        //send_sms("+84778786442",message);
    }
    }



// Task nhận dữ liệu từ module SIM
void rx_task(void *arg) {
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
    while (1) {
        int len = uart_read_bytes(SIM_UART_NUM, data, BUF_SIZE - 1, pdMS_TO_TICKS(1000));
        if (len > 0) {
            data[len] = '\0'; // Kết thúc chuỗi
            ESP_LOGI(TAG, "Received: %s", data);
            process_sms_response((char*) data);
        }
    }
    free(data);
}

void module_sim(void) {
    // Cấu hình UART cho module SIM
    const uart_config_t uart_config = {
        .baud_rate = UART_SIM_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(SIM_UART_NUM, &uart_config);
    uart_set_pin(SIM_UART_NUM, UART_SIM_TX_PIN, UART_SIM_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(SIM_UART_NUM, BUF_SIZE, 0, 0, NULL, 0);
    xTaskCreate(rx_task, "rx_task", 4096, NULL, 3, NULL);
    // Cấu hình module SIM để nhận tin nhắn
    send_at_command("AT");
    send_at_command("ATI");
    send_at_command("AT+CPIN?");
    send_at_command("AT+CSQ");
    send_at_command("AT+CIMI");
    send_at_command("AT+CNMI=2,2,0,0,0");
    send_at_command("AT+CMGD=1,4");
    vTaskDelay(pdMS_TO_TICKS(5000));
    // Khởi chạy task nhận dữ liệu
    
}
void call_back_uart(void* f){
    isr1 = f;
}