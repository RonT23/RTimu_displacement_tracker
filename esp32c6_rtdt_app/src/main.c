/*
 * Project: Real-Time Displacement Monitoring Unit
 * File: main.c
 * Author: Ronaldo Tsela
 * Date: July 2025
 * 
 * This is the main application entry point for the real-time displacement monitoring unit.
 * It was developed as part of the final project in the "Multivariable Control Systems" course,
 * within the MSc in Control Systems and Robotics program at the National Technical University of Athens (NTUA).
 * 
 * - This application is designed to run on the ESP32 C6.
 * - Developed using the ESP-IDF v5.4.2 framework.
 * - Uses FreeRTOS for application life cycle and execution management.
 * - Requires the MPU6050 IMU. 
 * - It includes initialization routines, data acquisition logic,
 *   and real-time processing for displacement retrival.
 * - The output is printed in CSV format through the main USB to Serial interface
 * - Read the provided documentation for more details.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app_tasks.h"
#include "mpu6050.h"

void app_main(void)
{
    esp_err_t res;

    // start with default values
    task_config_t task_cfg; 

    task_cfg.update_rate_ms    = 50;
    task_cfg.accel_noise_floor = 0.5;
    task_cfg.cfg = (mpu6050_config_t){
        .accel_range = 0,      // +-2g
        .gyro_range  = 0,      // +-250 deg/sec
        .dlpf_cfg    = 3,      // DLPF bandwidth: ~44Hz accel / ~42Hz gyro
        .smplrt_div  = 0       // Sample rate = 1kHz / (1 + 0) = 1kHz
    };

    // initialize the I2C interface
    res = i2c_master_init(); 
    if (res != ESP_OK) {
        ESP_LOGE("System", "I2C Initialization failed: %s", esp_err_to_name(res));
        vTaskSuspend(NULL);
    }

    // initialize the MPU6050 device
    res = mpu6050_init(I2C_NUM_0);
    if (res != ESP_OK) {
        ESP_LOGE("System", "MPU6050 Initialization failed: %s", esp_err_to_name(res));
        vTaskSuspend(NULL);
    }

    // configure MPU6050 device
    res = mpu6050_config(I2C_NUM_0, &task_cfg.cfg);
    if (res != ESP_OK) {
        ESP_LOGI("System", "MPU6050 Configuration failed: %s", esp_err_to_name(res));
        vTaskSuspend(NULL);
    }

    ESP_LOGI("System", "Initialized");

    // start the system monitor task
    xTaskCreate(
        system_monitor_task,   
        "system_monitor",       
        2048,                 
        NULL,                  
        5,              
        NULL         
    );                  

    // start the sensor readout tasl
    xTaskCreate(
        accel_readout_task,
        "accel_readout",
        2048,
        &task_cfg,
        2,
        NULL
    );
}
