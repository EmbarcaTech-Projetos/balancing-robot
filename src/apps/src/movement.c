#include "movement.h"
#include <stdio.h>

const static float dt=0.1;
const static int limite = 50000;
static float estimated_ang=0; 
static float integral=0;

float _get_angle(hal_imu_data_t *imu_data, hal_imu_data_t *initial_imu_data, float cte);
float _get_derivative(hal_imu_data_t *imu_data);
float _get_integral(float angle);
float _complementary_filter(float estimated_ang_acel, float gyro, float cte);

void imu_data_process(hal_imu_data_t *imu_data, hal_imu_data_t *initial_imu_data, constants *constants_variable)
{
    float kp = constants_variable->kp;
    float kd = constants_variable->kd;
    float ki = constants_variable->ki;

    uint16_t deadzone = constants_variable->deadzone;

    float accel_x = imu_data->accel_x_g - initial_imu_data->accel_x_g;
    float accel_y = imu_data->accel_y_g - initial_imu_data->accel_y_g;
    float accel_z = imu_data->accel_z_g - initial_imu_data->accel_z_g;

    float angle = _get_angle(imu_data, initial_imu_data, constants_variable->complementary_filter_alpha);
    float derivate = _get_derivative(imu_data);
    integral = _get_integral(angle);
    // Speed
    float x = angle*kp + derivate*kd + integral*ki;

    // For debugging
    printf("{\n");
    printf("  Raw Accel: %f %f %f\n", imu_data->accel_x_g, imu_data->accel_y_g, imu_data->accel_z_g);
    printf("  Accel: %f %f %f\n", accel_x, accel_y, accel_z);
    printf("  Gyro: %f %f %f\n", imu_data->gyro_x_dps, imu_data->gyro_y_dps, imu_data->gyro_z_dps);
    printf("  Angle: %f\n", angle);
    printf("  Derivate: %f\n", derivate);
    printf("  Integral: %f\n", integral);
    printf("  Speed: %f\n", x);
    printf("}\n");

    hal_motor_set_speeds(x, -x, deadzone);
}

/**
 * @brief Returns the current angle of the robot (using the complementary filter).
 * @param imu_data All measurements taken from the IMU.
 * @param initial_imu_data All measurements taken from the IMU when the robot connected to the Wi-Fi.
 * @param cte Constant between 1.0 and 0.0 measuring the contribution of the gyroscope.
 * @return Weighted estimated angle between accelorem.
 */
float _get_angle(hal_imu_data_t *imu_data, hal_imu_data_t *initial_imu_data, float cte)
{
    float accel_x = imu_data->accel_x_g - initial_imu_data->accel_x_g;
    return _complementary_filter(accel_x, imu_data->gyro_x_dps, cte);
}

/**
 * @brief Get the derivative of the angle from the robot.
 * @param imu_data All measurements taken from the IMU.
 * @return Returns derivative (speed) of the robot.
 */
float _get_derivative(hal_imu_data_t *imu_data)
{
    return imu_data->gyro_x_dps;
}

/**
 * @brief Integrate the angle and adds the integral to a global variable.
 * @param angle Current angle of the robot.
 * @return Returns the integral over the entire time.
 */
float _get_integral(float angle)
{
    integral += angle*dt;
    if (integral > limite) integral = limite;
    else if (integral < -limite) integral = -limite;
    return integral;
}

/**
 * @brief Implement a Complementary Filter to fuse accelerometer and gyroscope data.
 * @param estimated_ang_acel Angle measured by the acceloremeter
 * @param gyro Gyroscope speed in degrees per second
 * @return Weighted estimated angle between acceloremeter and gyroscope.
 */

float _complementary_filter(float estimated_ang_acel, float gyro, float cte)
{
    return (1-cte)*(estimated_ang+gyro*dt) + cte*(estimated_ang_acel);
}