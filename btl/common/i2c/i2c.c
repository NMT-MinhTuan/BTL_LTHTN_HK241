#include "i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           32               // Chân SCL của ESP32
#define I2C_MASTER_SDA_IO           33               // Chân SDA của ESP32
#define I2C_MASTER_NUM              I2C_NUM_0        // Sử dụng I2C port 0
#define I2C_MASTER_FREQ_HZ          100000           // Tần số xung nhịp I2C
#define I2C_MASTER_TX_BUF_DISABLE   0                // Tắt bộ đệm TX
#define I2C_MASTER_RX_BUF_DISABLE   0                // Tắt bộ đệm RX
#define I2C_MASTER_TIMEOUT_MS       1000
#define SHT30_SENSOR_ADDR           0x44             // Địa chỉ I2C của SHT30
static const char *TAG = "SHT300";

void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_TX_BUF_DISABLE, I2C_MASTER_RX_BUF_DISABLE, 0));
}

esp_err_t sht30_start_measurement() {
    uint8_t cmd[] = {0x2C, 0x06};
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SHT30_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, cmd, sizeof(cmd), true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

esp_err_t sht30_read_temperature_humidity(float *temperature, float *humidity) {
    uint8_t data[6]; // 2 bytes nhiệt độ, 2 bytes độ ẩm, 2 bytes CRC
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SHT30_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle, data, sizeof(data) - 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd_handle, data + 5, I2C_MASTER_NACK);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd_handle);

    if (ret == ESP_OK) {
        // Tính toán nhiệt độ và độ ẩm từ dữ liệu đọc được
        uint16_t temp_raw = (data[0] << 8) | data[1];
        uint16_t hum_raw = (data[3] << 8) | data[4];

        *temperature = -45.0 + 175.0 * ((float) temp_raw / 65535.0);
        *humidity = 100.0 * ((float) hum_raw / 65535.0);
    }
    return ret;
}

float task_i2c(char c){
    
    //i2c_master_init();
    
        esp_err_t ret = sht30_start_measurement();
        float temperature, humidity;
        if (ret == ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(500));  // Chờ dữ liệu sẵn sàng
            
            ret = sht30_read_temperature_humidity(&temperature, &humidity);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Temperature: %.2f C, Humidity: %.2f %%", temperature, humidity);
                if(c == 'T'){
                    return temperature;
                }else if (c == 'H'){
                    return humidity;
                }
            } else {
                ESP_LOGE(TAG, "Failed to read data from SHT30");
            }
        } else {
            ESP_LOGE(TAG, "Failed to start measurement");
        }
    return 0.0;
}


// void init_measure(void){
//     xTaskCreate(task_i2c, "task_i2c", 2048, NULL, 1, NULL);
// }