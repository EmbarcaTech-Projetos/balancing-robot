#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For atof()

#include "pico/cyw43_arch.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"

#include "hal.h"

// A buffer to store the incoming JSON data
static char json_buffer[256];
static int json_buffer_len = 0;

static hal_imu_data_t imu_data;
static float kp = 0.f;

/**
 * @brief This function is called when a POST request begins.
 */
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd)
{
    // We only want to handle POST requests to "/command"
    if (strcmp(uri, "/command") == 0) {
        // Clear the buffer for new data
        memset(json_buffer, 0, sizeof(json_buffer));
        json_buffer_len = 0;

        // Check if the data will fit in our buffer
        if (content_len <= sizeof(json_buffer)) {
            *post_auto_wnd = 1; // Let lwIP manage the TCP window
            return ERR_OK;
        }
    }

    return ERR_VAL;
}

/**
 * @brief This function is called with chunks of POST data.
 */
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
    if (p != NULL) {
        // Check for buffer overflow
        if (json_buffer_len + p->len <= sizeof(json_buffer)) {
            memcpy(json_buffer + json_buffer_len, p->payload, p->len);
            json_buffer_len += p->len;
        }
        // Free the pbuf
        pbuf_free(p);
    }
    return ERR_OK;
}

/**
 * @brief This function is called when the POST request is finished.
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
    printf("POST request finished. Received data:\n%s\n", json_buffer);

    // --- Simple JSON "Parsing" ---

    char* kp_key = strstr(json_buffer, "\"kp\":");
    if (kp_key) {
        kp = atof(kp_key + strlen("\"kp\":"));
        printf("KP constant change: %.2f\n", kp);
    }
    
    // Set the response URI. Redirecting back to the main page.
    strncpy(response_uri, "/index.shtml", response_uri_len);
}

void imu_data_process(hal_imu_data_t *imu_data)
{
    float x = imu_data->accel_x_g * kp;
    printf("%d\n", x);
    hal_motor_set_speeds(x, -x);
}

int main() {
    if (!hal_init("XXXX", "XXXX")) {
        while(1) {
            hal_set_onboard_led(true);
            sleep_ms(100);
            hal_set_onboard_led(false);
            sleep_ms(100);
        }
    }

    hal_http_server_init();

    printf("HTTP Server started. IP Address: %s\n", hal_wifi_get_ip_address_str());
    
    int led_counter = 0;
    while (true) {

        hal_imu_get_data(&imu_data);
        imu_data_process(&imu_data);

        if(led_counter++ % 10 == 0) {
            hal_toggle_onboard_led();
            printf("Send POST requests to http://%s/command\n", hal_wifi_get_ip_address_str());
        }
        sleep_ms(100);
    }

    return 0;
}