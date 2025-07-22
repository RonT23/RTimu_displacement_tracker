#include "app_tasks.h"

esp_err_t i2c_master_init(void) {
    // Configure the I2C interface
    i2c_config_t conf = {
        .mode               = I2C_MODE_MASTER,
        .sda_io_num         = I2C_MASTER_SDA_IO,
        .scl_io_num         = I2C_MASTER_SCL_IO,
        .sda_pullup_en      = GPIO_PULLUP_ENABLE,
        .scl_pullup_en      = GPIO_PULLUP_ENABLE,
        .master.clk_speed   = 400000,
        .clk_flags          = 0,
    };

    // Apply the configurations
    esp_err_t res = i2c_param_config(I2C_NUM_0, &conf);
    if (res != ESP_OK) { return res; } 

    // Install the driver.
    // This function allocates internal data structures and hooks up the driver, 
    // so we can use 
    // i2c_master_write_to_device(...);
    // i2c_master_read_from_device(...);
    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

void accel_readout_task(void *pvParameters) {
    task_config_t *config = (task_config_t *)pvParameters;

    esp_err_t res;

    mpu6050_data_t accel_data;
    mpu6050_cal_data_t accel_bias_data;
    motion_state_t state; 
 
    // calibrate the accelerometer readings first
    res = mpu6050_calibrate_accel(I2C_NUM_0, &accel_bias_data);
    if ( res == ESP_OK) {
        ESP_LOGI("Calibration", "%.2f, %.2f, %.2f", accel_bias_data.ax_bias, accel_bias_data.ay_bias, accel_bias_data.az_bias);
    } else {
        ESP_LOGE("Calibration", "Failed: %s", esp_err_to_name(res));
        vTaskSuspend(NULL);
    }

    int64_t last_time = esp_timer_get_time();

    while (1) {
        if (config->start) {

            // Read acceleration
            res = mpu6050_read_accel(I2C_NUM_0, &accel_data);

            // measure integration time difference
            int64_t now = esp_timer_get_time(); // in microseconds
            float dt = (now - last_time) / 1e6f;
            last_time = now;

            process_accel_data(accel_data, accel_bias_data, config->accel_noise_floor, dt, &state);

            if (res == ESP_OK) {
                ESP_LOGI("Acceleration", "%.2f,%.2f,%.2f", state.ax, state.ay, state.az);
                ESP_LOGI("Velocity", "%.2f,%.2f,%.2f", state.vx, state.vy, state.vz);
                ESP_LOGI("Displacement", "%.2f,%.2f,%.2f", state.dx, state.dy, state.dz);
            } else {
                ESP_LOGW("ReadOut", "Failed: %s", esp_err_to_name(res));
            }

        } 
        vTaskDelay(pdMS_TO_TICKS(config->update_rate_ms)); // update rate
    }
}

void process_accel_data(mpu6050_data_t data, mpu6050_cal_data_t bias, float noise_threshold, float dt, motion_state_t *state) {
    // Bias Compensation
    float ax = data.ax - bias.ax_bias;
    float ay = data.ay - bias.ay_bias;
    float az = data.az - bias.az_bias;

    // Noise Filtering
    if (fabs(ax) < noise_threshold) ax = 0;
    if (fabs(ay) < noise_threshold) ay = 0;
    if (fabs(az) < noise_threshold) az = 0;

    // Stillness Detection with Hold Time 
    float stationary_threshold = 0.05;
    int hold_cycles = 10;  // Number of consecutive samples to confirm stillness

    // Persistent counters for each axis
    static int ax_still_count = 0;
    static int ay_still_count = 0;
    static int az_still_count = 0;

    // X-axis
    if (fabs(ax) < stationary_threshold) {
        ax_still_count++;
        if (ax_still_count >= hold_cycles) {
            state->vx = 0;
        }
    } else {
        ax_still_count = 0;
        state->vx += ax * dt;
        state->dx += state->vx * dt;
    }

    // Y-axis
    if (fabs(ay) < stationary_threshold) {
        ay_still_count++;
        if (ay_still_count >= hold_cycles) {
            state->vy = 0;
        }
    } else {
        ay_still_count = 0;
        state->vy += ay * dt;
        state->dy += state->vy * dt;
    }

    // Z-axis
    if (fabs(az) < stationary_threshold) {
        az_still_count++;
        if (az_still_count >= hold_cycles) {
            state->vz = 0;
        }
    } else {
        az_still_count = 0;
        state->vz += az * dt;
        state->dz += state->vz * dt;
    }

    // Store acceleration in state
    state->ax = ax;
    state->ay = ay;
    state->az = az;
}

void system_monitor_task(void *pvParameters) {
    uint8_t reg;
    uint8_t who_am_i = 0;
    esp_err_t res;

    uint32_t start_time_sec = esp_timer_get_time() / 1000000;

    while (1) {
        uint32_t now_sec = esp_timer_get_time() / 1000000;
        uint32_t uptime = now_sec - start_time_sec;


        ESP_LOGI("SystemMonitor", "Uptime=%lu s", uptime);

        // MPU6050 WHO_AM_I check via I2C
        reg = MPU6050_WHO_AM_I;
        res = i2c_master_write_read_device(I2C_NUM_0, MPU6050_ADDR, &reg, 1, &who_am_i, 1, pdMS_TO_TICKS(1000));
        if (res == ESP_OK && who_am_i == MPU6050_DEVICE_ID) {
            ESP_LOGI("SystemMonitor", "MPU6050 OK");
        } else {
            ESP_LOGE("SystemMonitor", "MPU6050 not responding (%s)", esp_err_to_name(res));
        }

        // Heap Status
        size_t heap_free = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        ESP_LOGI("SystemMonitor", "Heap free=%u bytes", heap_free);

        vTaskDelay(pdMS_TO_TICKS(30000)); // repeat every 30 seconds
    }
}

void command_listener_task(void *pvParameters) {
    char buf[CMD_BUF_SIZE];
    task_config_t *config = (task_config_t *)pvParameters;

    while (1) {
        if (fgets(buf, CMD_BUF_SIZE, stdin) != NULL) {
            buf[strcspn(buf, "\r\n")] = 0; 

            ESP_LOGI("CommandListener", "Received: %s", buf);

            if (strncmp(buf, "reset", 5) == 0) {
                esp_restart();
            
            } else if (strncmp(buf, "set_rate:", 9) == 0) {
            
                int rate = atoi(buf + 9);
                config->update_rate_ms = rate;
                ESP_LOGI("CommandListener", "Set Update Rate: %d", rate);
            
            }  else if (strncmp(buf, "set_accel_noise_floor:", 23) == 0) {
            
                float noise = atof(buf + 23);
                config->accel_noise_floor = noise;
                ESP_LOGI("CommandListener", "Set Accel. Noice Floor: %.2f", noise);
            
            }  else if (strncmp(buf, "set_mpu6050_config:", 19) == 0) {
                int a, g, d, s;

                if (sscanf(buf + 19, "%d,%d,%d,%d", &a, &g, &d, &s) == 4) {
                    config->cfg.accel_range = a;
                    config->cfg.gyro_range  = g;
                    config->cfg.dlpf_cfg    = d;
                    config->cfg.smplrt_div  = s;

                    mpu6050_config(I2C_NUM_0, &config->cfg);
                    ESP_LOGI("CommandListener", "MPU6050 reconfigured: a=%d, g=%d, d=%d, s=%d", a, g, d, s);
                } else {
                    ESP_LOGE("CommandListener", "Invalid config string");
                }

            } else if (strncmp(buf, "start", 5) == 0) {
                config->start = true;
                ESP_LOGI("CommandListener", "Starting the readout task");
            } else if (strncmp(buf, "stop", 4) == 0) {
                config->start = false;
                ESP_LOGI("CommandListener", "Stoping the readout taks");
            } else {
                ESP_LOGE("CommandListener", "Unknown command: %s\n", buf);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
