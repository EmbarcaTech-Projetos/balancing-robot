#include <stdio.h>
#include "pico/stdlib.h"
#include "motor.h" 

int main() {
    stdio_init_all();
    motor_setup();
    motor_enable();

    // 50% power forward
    uint16_t power = (uint16_t)(0.50 * 255) << 8;
    motor_set_both_level(power, false); // false = forward

    sleep_ms(10000); // Run the motors for 10 seconds

    motor_set_both_level(0, false); // stop

    while (true) {
        tight_loop_contents(); // Keep the program running
    }
}