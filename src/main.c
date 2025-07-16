#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "mpu6050_i2c.h" 

int main() {
    stdio_init_all();
    mpu6050_setup_i2c();
    mpu6050_reset();

    sleep_ms(1000); // Give time to open Serial Monitor

    int16_t accel_raw[3], gyro_raw[3], temp;
    float accel[3], gyro[3];

    printf("Starting accelerometer readings...\n");

    while (true) {
        // Read raw accel data
        mpu6050_read_raw(accel_raw, gyro_raw, &temp);

        // Convert to g's and degrees per second
       for (uint8_t i = 0; i < 3; i++) {
            accel[i] = (float)accel_raw[i] / 16384.0f;
            gyro[i] = (float)gyro_raw[i] / 131.0f;
       }

       // Print
       printf("Rotation X: %.3f Y: %.3f Z: %.3f\n", accel[0], accel[1], accel[2]);
       printf("Angular Velocity X: %.3f Y: %.3f Z: %.3f\n", gyro[0], gyro[1], gyro[2]);

       sleep_ms(1000); // Adjust as needed for readability
    }
}