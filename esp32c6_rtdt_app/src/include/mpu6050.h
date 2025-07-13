#ifndef MPU6050_H 
#define MPU6050_H 

#include "driver/i2c.h"     
#include "driver/gpio.h"    
#include "esp_err.h"        
#include "esp_log.h"        

// --- MPU6050 Device Constants ---

#define MPU6050_ADDR         0x68    // Default I2C address of MPU6050
#define MPU6050_DEVICE_ID    0x68    // Expected WHO_AM_I register value
#define MPU6050_CLKSEL_PLL   0x01    // Clock source: X-axis gyroscope PLL
#define MPU6050_WAKE_UP      0x00    // Command to wake the sensor from sleep

// --- Register Map Addresses ---

#define MPU6050_WHO_AM_I     0x75    // WHO_AM_I register (device ID)
#define MPU6050_PWR_MGMT_1   0x6B    // Power management register

#define MPU6050_SMPLRT_DIV   0x19    // Sample rate divider register
#define MPU6050_CONFIG       0x1A    // Configuration register (DLPF, FSYNC)
#define MPU6050_GYRO_CONFIG  0x1B    // Gyroscope range configuration
#define MPU6050_ACCEL_CONFIG 0x1C    // Accelerometer range configuration

#define MPU6050_ACCEL_XOUT_H 0x3B    // Start of accelerometer data (6 bytes total: X, Y, Z, each in H and L)
#define MPU6050_GYRO_XOUT_H  0x43    // Start of gyroscope data (6 bytes total: X, Y, Z, each in H and L)
#define MPU6050_TEMP_OUT_H   0x41    // Temperature data high byte (2 bytes total: H, L)

// --- Scaling Factors ---

#define ACCEL_SCALE (9.80665 / 16384.0) // Convert raw accel data (LSB) to m/s² for ±2g
#define GYRO_SCALE  (1.0 / 131.0)       // Convert raw gyro data (LSB) to °/s for ±250°/s

// --- Configuration Structure ---

/**
 * @brief Configuration settings for the MPU6050
 */
typedef struct {
    uint8_t accel_range;   // Accelerometer range: 0=±2g, 1=±4g, 2=±8g, 3=±16g
    uint8_t gyro_range;    // Gyroscope range: 0=±250, 1=±500, 2=±1000, 3=±2000 °/s
    uint8_t dlpf_cfg;      // Digital Low Pass Filter setting (0–6)
    uint8_t smplrt_div;    // Sample rate divider: SampleRate = 1kHz / (1 + smplrt_div)
} mpu6050_config_t;

// --- Sensor Data Output Structure ---

/**
 * @brief Output data from the MPU6050 (accel, gyro, temp)
 */
typedef struct {
    float ax;   // Acceleration in X (m/s²)
    float ay;   // Acceleration in Y (m/s²)
    float az;   // Acceleration in Z (m/s²)
    float gx;   // Angular velocity in X (°/s)
    float gy;   // Angular velocity in Y (°/s)
    float gz;   // Angular velocity in Z (°/s)
    float temp; // Temperature in °C
} mpu6050_data_t;

// --- Calibration Data Structure ---

/**
 * @brief Biases determined during calibration
 */
typedef struct {
    int samples;      // Number of samples used in calibration
    float ax_bias;   
    float ay_bias;
    float az_bias;
    float gx_bias;
    float gy_bias;
    float gz_bias;
} mpu6050_cal_data_t;

// --- MPU6050 API Functions ---

/**
 * @brief Initialize the MPU6050 sensor (wake from sleep and check WHO_AM_I)
 * 
 * @param port I2C port (e.g., I2C_NUM_0)
 * @return esp_err_t ESP_OK on success or error code on failure
 */
esp_err_t mpu6050_init(i2c_port_t port);

/**
 * @brief Configure the MPU6050 with specified range and filtering settings
 * 
 * @param port I2C port
 * @param cfg Pointer to mpu6050_config_t struct
 * @return esp_err_t ESP_OK or error code on failure
 */
esp_err_t mpu6050_config(i2c_port_t port, const mpu6050_config_t *cfg);

/**
 * @brief Read and convert accelerometer data
 * 
 * @param port I2C port
 * @param data Output struct to store accel data
 * @return esp_err_t ESP_OK or error code
 */
esp_err_t mpu6050_read_accel(i2c_port_t port, mpu6050_data_t *data);

/**
 * @brief Read and convert gyroscope data
 * 
 * @param port I2C port
 * @param data Output struct to store gyro data
 * @return esp_err_t ESP_OK or error code
 */
esp_err_t mpu6050_read_gyro(i2c_port_t port, mpu6050_data_t *data);

/**
 * @brief Read and convert temperature data
 * 
 * @param port I2C port
 * @param data Output struct to store temperature
 * @return esp_err_t ESP_OK or error code
 */
esp_err_t mpu6050_read_temp(i2c_port_t port, mpu6050_data_t *data);

/**
 * @brief Calibrate accelerometer by computing bias offsets
 * 
 * @param port I2C port
 * @param cal_data Pointer to store computed calibration offsets
 * @return esp_err_t ESP_OK or error code
 */
esp_err_t mpu6050_calibrate_accel(i2c_port_t port, mpu6050_cal_data_t *cal_data);

#endif // MPU6050_H
