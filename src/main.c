#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "motor.h"
#include "mpu6050_i2c.h" 

// Emergency button pin
#define BUTTON_B 6

int main() {
    // Setup
    stdio_init_all();
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    mpu6050_setup_i2c();
    mpu6050_reset();
    motor_setup();
    motor_enable();

    // Variables for IMU
    int16_t accel_raw[3], gyro_raw[3], temp;
    float accel[3], gyro[3];

    uint16_t power = (uint16_t)(0.35 * 255) << 8;
    motor_set_both_level(power, false); // false = forward

    while (true) {

        // Read IMU
        mpu6050_read_raw(accel_raw, gyro_raw, &temp);

        for (uint8_t i = 0; i < 3; ++i) {
            accel[i] = (float)accel_raw[i] / 16384.0f;
            gyro[i] = (float)gyro_raw[i] / 131.0f;
        }

        // Check emergency stop by sensor high values or button
        bool high_accel = (accel[2] > 0.450f) || (accel[2] < -0.450f);
        bool high_gyro = (gyro[1] > 420.0f) || (gyro[1] < -420.0f);
        bool emergency_button = !gpio_get(BUTTON_B); // Active low
        sleep_ms(25); // let microcontroller rest between readings

        if (high_accel || high_gyro || emergency_button) {
            motor_set_both_level(0, true); // Stop motors with level=0
            // motor_disable(); // the motor wont enable again automatically
            sleep_ms(4000); // Give the user 4 seconds to solve the emergency
            motor_set_both_level(power,false);
        }
    }
}
























// int main() {
//     stdio_init_all();
//     motor_setup();
//     motor_enable();

//     // 50% power forward
//     uint16_t power = (uint16_t)(0.50 * 255) << 8;
//     motor_set_both_level(power, false); // false = forward

//     sleep_ms(10000); // Run the motors for 10 seconds

//     motor_set_both_level(0, false); // stop

//     while (true) {
//         tight_loop_contents(); // Keep the program running
//     }
// }