#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "motor.h"
#include "mpu6050_i2c.h" 
#include "math.h"

int main() {
    stdio_init_all();
    mpu6050_setup_i2c();
    mpu6050_reset();
    motor_setup();
    // Here you can blink a led or print to indicate that program flashed and is running

    int16_t accel_raw[3], gyro_raw[3], temp;
    float accel[3];

    while (true) {
        mpu6050_read_raw(accel_raw, gyro_raw, &temp);

        for (uint8_t i = 0; i < 3; ++i) {
            accel[i] = (float)accel_raw[i] / 16384.0f;
        }


        // Threshold test on pitch (example using X-axis)
        // Adjust threshold as needed (e.g., 0.5g ~ 30º)
        // Inside the if/else you can blink led or print, so you have a feedback
        if (fabsf(accel[0]) > 0.5f) {
            motor_enable();
            bool direction_test = accel[0] < 0; // direction based on tilt
            motor_set_both_level(100 << 8, direction_test);
         }
        else { motor_set_both_level(0, false); }


        sleep_ms(100);
    }
}