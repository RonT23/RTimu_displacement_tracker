#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/i2c.h" 

#include "mpu6050.h"

#define CMD_BUF_SIZE                128

#define I2C_MASTER_SDA_IO           19      // GPIO for Master I2C data line (SDA)
#define I2C_MASTER_SCL_IO           20      // GPIO for Master I2C clock line (SCL)

/**
 * @brief Task configuration structure for FreeRTOS tasks
 *
 * update_rate_ms:       Period at which sensor data is updated (in milliseconds)
 * accel_noise_floor:    Threshold below which accelerometer data is considered noise
 * cfg:                  Configuration parameters for MPU6050
 */
typedef struct {
    uint32_t update_rate_ms;
    float accel_noise_floor;
    bool start;
    mpu6050_config_t cfg;
} task_config_t;

typedef struct {
    float ax, ay, az;  // acceleration in m/s²
    float vx, vy, vz;  // velocity in m/s
    float dx, dy, dz;  // displacement in meters
} motion_state_t;

/**
 * @brief Initialize the I2C master interface
 *
 * @return esp_err_t     ESP_OK on success, error code on failure
 */
esp_err_t i2c_master_init(void);

/**
 * @brief Task to periodically read accelerometer and gyroscope data
 *
 * This task polls data from the MPU6050 sensor based on the configuration
 * provided in the task_config_t structure. Data is filtered and processed.
 *
 * @param pvParameters   Pointer to task_config_t passed during task creation
 */
void accel_readout_task(void *pvParameters);


/**
 * @brief Process raw accelerometer data: bias compensation, noise filtering,
 *        and numerical integration to compute velocity and displacement.
 *
 * @param data              Raw accelerometer readings (m/s²)
 * @param bias              Bias offset from calibration
 * @param noise_threshold   Acceleration threshold below which noise is ignored
 * @param dt                Time step between samples (in seconds)
 * @param state             Pointer to persistent motion state (velocity/displacement)
 */
void process_accel_data(mpu6050_data_t data, mpu6050_cal_data_t bias, float noise_threshold, float dt, motion_state_t *state);

/**
 * @brief Task to monitor system health and diagnostics 
 *
 * Used to monitor memory usage, runtime stats, and periodic logging
 * for system diagnostics.
 *
 * @param pvParameters   Unused here
 */
void system_monitor_task(void *pvParameters);

void command_listener_task(void *pvParameters);

#endif // APP_TASKS_H
