#include "movement.h"
#include <stdio.h>

void imu_data_process(hal_imu_data_t *imu_data, hal_imu_data_t *initial_imu_data, constants *constants_variable)
{
    float kp = constants_variable->kp;
    float kd = constants_variable->kd;
    float ki = constants_variable->ki;

    uint16_t deadzone = constants_variable->deadzone;

    float accel_x = imu_data->accel_x_g - initial_imu_data->accel_x_g;
    float accel_y = imu_data->accel_y_g - initial_imu_data->accel_y_g;
    float accel_z = imu_data->accel_z_g - initial_imu_data->accel_z_g;

    float x = accel_x * kp;

    printf("{\n");
    printf("  Raw Accel: %f %f %f\n", imu_data->accel_x_g, imu_data->accel_y_g, imu_data->accel_z_g);
    printf("  Accel: %f %f %f\n", accel_x, accel_y, accel_z);
    printf("  Gyro: %f %f %f\n", imu_data->gyro_x_dps, imu_data->gyro_y_dps, imu_data->gyro_z_dps);
    printf("  Speed: %f\n", x);
    printf("}\n");

    hal_motor_set_speeds(x, -x, deadzone);
}