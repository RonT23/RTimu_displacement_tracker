idf_component_register(
    SRCS         "main.c" "app_tasks.c" "mpu6050.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES driver esp_timer
)
