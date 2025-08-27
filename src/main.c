#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "hal.h"
#include "network.h"
#include "movement.h"

static constants constants_variable;
static hal_imu_data_t imu_data;
static hal_imu_data_t initial_imu_data;

int main() {
    if (!hal_init("REDEDOMARCOS", "45612300")) {
        while(1) {
            hal_set_onboard_led(true);
            sleep_ms(100);
            hal_set_onboard_led(false);
            sleep_ms(100);
        }
    }

    hal_http_server_init();

    printf("HTTP Server started. IP Address: %s\n", hal_wifi_get_ip_address_str());
    hal_imu_get_data(&initial_imu_data);

    int led_counter = 0;
    while (true) {
        hal_imu_get_data(&imu_data);

        constants_variable.deadzone = deadzone;
        constants_variable.kp = kp;
        constants_variable.kd = kd;
        constants_variable.ki = ki;
        constants_variable.complementary_filter_alpha = complementary_filter_alpha;

        imu_data_process(&imu_data, &initial_imu_data, &constants_variable);

        if(led_counter++ % 10 == 0) {
            hal_toggle_onboard_led();
            printf("Send POST requests to http://%s/command\n", hal_wifi_get_ip_address_str());
        }
        sleep_ms(100);
    }

    return 0;
}