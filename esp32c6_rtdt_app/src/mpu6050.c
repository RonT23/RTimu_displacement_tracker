#include "mpu6050.h"

esp_err_t mpu6050_config(i2c_port_t port, const mpu6050_config_t *cfg) {
    esp_err_t res;
    uint8_t data[2];

    // SMPLRT_DIV
    data[0] = MPU6050_SMPLRT_DIV;
    data[1] = cfg->smplrt_div;
    res = i2c_master_write_to_device(port, MPU6050_ADDR, data, 2, pdMS_TO_TICKS(100));
    if (res != ESP_OK) { return res; }

    // CONFIG
    data[0] = MPU6050_CONFIG;
    data[1] = cfg->dlpf_cfg;
    res = i2c_master_write_to_device(port, MPU6050_ADDR, data, 2, pdMS_TO_TICKS(100));
    if (res != ESP_OK) { return res; }

    // GYRO_CONFIG
    data[0] = MPU6050_GYRO_CONFIG;
    data[1] = cfg->gyro_range << 3;
    res = i2c_master_write_to_device(port, MPU6050_ADDR, data, 2, pdMS_TO_TICKS(100));
    if (res != ESP_OK) { return res; }

    // ACCEL_CONFIG
    data[0] = MPU6050_ACCEL_CONFIG;
    data[1] = cfg->accel_range << 3;
    res = i2c_master_write_to_device(port, MPU6050_ADDR, data, 2, pdMS_TO_TICKS(100));
    if (res != ESP_OK) { return res; }

    return ESP_OK;
}

esp_err_t mpu6050_init(i2c_port_t port) {
    uint8_t who_am_i = 0;
    uint8_t reg = MPU6050_WHO_AM_I;
    esp_err_t res;

    // WHO_AM_I check
    res = i2c_master_write_read_device(port, MPU6050_ADDR, &reg, 1, &who_am_i, 1, pdMS_TO_TICKS(1000));
    if (res != ESP_OK) {  return res; }

    if (who_am_i != MPU6050_DEVICE_ID) { return ESP_FAIL; }

    // Wake up (clear sleep bit)
    uint8_t data[2] = {MPU6050_PWR_MGMT_1, MPU6050_WAKE_UP};
    res = i2c_master_write_to_device(port, MPU6050_ADDR, data, 2, pdMS_TO_TICKS(1000));
    if (res != ESP_OK) { return res; }

    // Set clock source to PLL with X axis gyroscope reference
    data[0] = MPU6050_PWR_MGMT_1;
    data[1] = MPU6050_CLKSEL_PLL;
    res = i2c_master_write_to_device(port, MPU6050_ADDR, data, 2, pdMS_TO_TICKS(1000));
    if (res != ESP_OK) { return res; }
    
    return ESP_OK;
}

esp_err_t mpu6050_read_accel(i2c_port_t port, mpu6050_data_t *data) {
    uint8_t reg = MPU6050_ACCEL_XOUT_H;
    uint8_t raw[6];
    int16_t ax, ay, az;

    esp_err_t res = i2c_master_write_read_device(port, MPU6050_ADDR, &reg, 1, raw, 6, pdMS_TO_TICKS(1000));
    if (res != ESP_OK) { return res; }

    ax = (int16_t)(raw[0] << 8 | raw[1]);
    ay = (int16_t)(raw[2] << 8 | raw[3]);
    az = (int16_t)(raw[4] << 8 | raw[5]);

    data->ax = (float)ax * ACCEL_SCALE;
    data->ay = (float)ay * ACCEL_SCALE;
    data->az = (float)az * ACCEL_SCALE;

    return ESP_OK;
}

esp_err_t mpu6050_read_gyro(i2c_port_t port, mpu6050_data_t *data) {
    uint8_t reg = MPU6050_GYRO_XOUT_H;
    uint8_t raw[6];
    int16_t gx, gy, gz;

    esp_err_t res = i2c_master_write_read_device(port, MPU6050_ADDR, &reg, 1, raw, 6, pdMS_TO_TICKS(1000));
    if (res != ESP_OK) { return res; }

    gx = (int16_t)(raw[0] << 8 | raw[1]);
    gy = (int16_t)(raw[2] << 8 | raw[3]);
    gz = (int16_t)(raw[4] << 8 | raw[5]);

    data->gx = gx * GYRO_SCALE;
    data->gy = gy * GYRO_SCALE;
    data->gz = gz * GYRO_SCALE;

    return ESP_OK;
}

esp_err_t mpu6050_read_temp(i2c_port_t port, mpu6050_data_t *data) {
    uint8_t reg = MPU6050_TEMP_OUT_H;
    uint8_t raw[2];
    int16_t temp_raw;

    esp_err_t res = i2c_master_write_read_device(port, MPU6050_ADDR, &reg, 1, raw, 2, pdMS_TO_TICKS(1000));
    if (res != ESP_OK) { return res; }

    temp_raw = (int16_t)(raw[0] << 8 | raw[1]);
    data->temp = (temp_raw / 340.0f) + 36.53f;

    return ESP_OK;
}

esp_err_t mpu6050_calibrate_accel(i2c_port_t port, mpu6050_cal_data_t *cal_data) {
    mpu6050_data_t accel_data;
    esp_err_t res;

    cal_data->samples = 100;
    cal_data->ax_bias = 0;
    cal_data->ay_bias = 0;
    cal_data->az_bias = 0;

    for (int i = 0; i < cal_data->samples; i++) {
        res = mpu6050_read_accel(port, &accel_data);
        if(res != ESP_OK) { return res; }

        cal_data->ax_bias += accel_data.ax;
        cal_data->ay_bias += accel_data.ay;
        cal_data->az_bias += accel_data.az;
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 100 Hz sampling
    }

    cal_data->ax_bias /= cal_data->samples;
    cal_data->ay_bias /= cal_data->samples;
    cal_data->az_bias /= cal_data->samples;

    return ESP_OK;
}