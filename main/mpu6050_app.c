#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mpu6050.h"

#define I2C_HOST I2C_NUM_0
#define I2C_SDA_GPIO GPIO_NUM_8
#define I2C_SCL_GPIO GPIO_NUM_9
#define SENSOR_READ_PERIOD_MS 1000

static const char *TAG = "mpu6050_app";

static i2c_master_bus_handle_t s_i2c_bus = NULL;
static mpu6050_handle_t s_mpu_handle;

static void init_i2c_bus(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_HOST,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &s_i2c_bus));
    ESP_LOGI(TAG, "Barramento I2C pronto: SDA=GPIO%d SCL=GPIO%d", I2C_SDA_GPIO, I2C_SCL_GPIO);
}

static void init_mpu6050(void)
{
    mpu6050_config_t sensor_config = MPU6050_DEFAULT_CONFIG();

    sensor_config.accel_fs = ACCEL_FS_4G;
    sensor_config.gyro_fs = GYRO_FS_500DPS;
    sensor_config.wake_auto = true;

    ESP_ERROR_CHECK(mpu6050_create(s_i2c_bus, MPU6050_DEFAULT_INFO(), &s_mpu_handle));
    ESP_ERROR_CHECK(mpu6050_config(s_mpu_handle, sensor_config));

    ESP_LOGI(TAG, "MPU6050 inicializado no endereco 0x%02X", MPU6050_DEFAULT_ADDR);
}

static void print_sensor_data(void)
{
    mpu6050_accel_value_t accel_value;
    mpu6050_gyro_value_t gyro_value;
    mpu6050_temp_value_t temp_value;

    ESP_ERROR_CHECK(mpu6050_get_accel(s_mpu_handle, &accel_value));
    ESP_ERROR_CHECK(mpu6050_get_gyro(s_mpu_handle, &gyro_value));
    ESP_ERROR_CHECK(mpu6050_get_temp(s_mpu_handle, &temp_value));

    printf("ACC[g] X=%.2f Y=%.2f Z=%.2f | GYRO[dps] X=%.2f Y=%.2f Z=%.2f | TEMP[C] %.2f\n",
           accel_value.accel_x,
           accel_value.accel_y,
           accel_value.accel_z,
           gyro_value.gyro_x,
           gyro_value.gyro_y,
           gyro_value.gyro_z,
           temp_value.temp);
}

void app_main(void)
{
    init_i2c_bus();
    init_mpu6050();

    while (true) {
        print_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_PERIOD_MS));
    }
}
