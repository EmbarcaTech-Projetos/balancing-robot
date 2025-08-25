/**
 * @file hal.c
 * @brief Implementation of the Hardware Abstraction Layer.
 */

#include "hal.h"
#include <math.h> // For fabs()

// Include the low-level drivers
#include "motor.h"
#include "mpu6050_i2c.h"

// Pico W specific includes for Wi-Fi and onboard LED
#include "pico/cyw43_arch.h"

#include "lwip/apps/httpd.h"
#include "lwip/netif.h" // For ip4addr_ntoa

// Sensitivity constant for the MPU6050 gyroscope at the default range (±250 dps).
// This value comes from the MPU6050 datasheet.
#define GYRO_SENS_250_DPS 131.0f

// Store the current accelerometer sensitivity. Default is for ±2g range.
static float accel_sensitivity = 16384.0f;

// A static variable to hold the user's callback function.
// A buffer to re-assemble the POST data from lwIP's parsed format.
static char post_data_buffer[256];

/**
 * @brief Initializes all hardware.
 */
bool hal_init(const char *wifi_ssid, const char *wifi_password) {
    // Initialize standard I/O for debugging.
    stdio_init_all();

    // Initialize the CYW43 Wi-Fi chip. Must be called first.
    if (cyw43_arch_init()) {
        printf("HAL Error: Failed to initialize cyw43_arch\n");
        return false;
    }

    // Initialize motor driver hardware.
    motor_setup();
    motor_enable();

    // Initialize I2C communication and the MPU6050 sensor.
    mpu6050_setup_i2c();
    mpu6050_reset();

    // Set a default accelerometer range (e.g., ±2g) and record its sensitivity.
    // The range can be changed later if needed.
    uint8_t accel_range_code = 0; // 0 = ±2g
    mpu6050_set_accel_range(accel_range_code);
    switch(accel_range_code) {
        case 0: accel_sensitivity = ACCEL_SENS_2G; break;
        case 1: accel_sensitivity = ACCEL_SENS_4G; break;
        case 2: accel_sensitivity = ACCEL_SENS_8G; break;
        case 3: accel_sensitivity = ACCEL_SENS_16G; break;
    }

    // Enable Station mode to connect to a Wi-Fi network.
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi network: %s\n", wifi_ssid);

    // Attempt to connect to the network.
    if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password, CYW43_AUTH_WPA2_AES_PSK, 80000)) {
        printf("HAL Error: Failed to connect to Wi-Fi\n");
        return false;
    } else {
        printf("Successfully connected to Wi-Fi\n");
    }

    return true;
}

/**
 * @brief Controls the onboard LED.
 */
void hal_set_onboard_led(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

/**
 * @brief Toggles the onboard LED.
 */
void hal_toggle_onboard_led() {
    static bool led_state = false;
    led_state = !led_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
}

/**
 * @brief Sets motor speeds based on a float from -1.0 to 1.0.
 */
void hal_motor_set_speeds(float speed_left, float speed_right) {
    // Clamp speeds to the valid range [-1.0, 1.0]
    if (speed_left > 1.0f) speed_left = 1.0f;
    if (speed_left < -1.0f) speed_left = -1.0f;
    if (speed_right > 1.0f) speed_right = 1.0f;
    if (speed_right < -1.0f) speed_right = -1.0f;

    // --- Left Motor ---
    bool forward_left = (speed_left >= 0);
    // Convert float speed to 16-bit PWM level
    uint16_t level_left = (uint16_t)(fabs(speed_left) * 65535.0f);
    motor_set_left_level(level_left, forward_left);

    // --- Right Motor ---
    bool forward_right = (speed_right >= 0);
    // Convert float speed to 16-bit PWM level
    uint16_t level_right = (uint16_t)(fabs(speed_right) * 65535.0f);
    motor_set_right_level(level_right, forward_right);
}

/**
 * @brief Stops both motors.
 */
void hal_motor_stop() {
    motor_set_both_level(0, true);
}

/**
 * @brief Reads and processes IMU data.
 */
bool hal_imu_get_data(hal_imu_data_t *data) {
    int16_t raw_accel[3], raw_gyro[3];
    int16_t raw_temp;

    // Read raw 16-bit integer values from the sensor.
    mpu6050_read_raw(raw_accel, raw_gyro, &raw_temp);

    // Convert raw accelerometer data to g's.
    data->accel_x_g = (float)raw_accel[0] / accel_sensitivity;
    data->accel_y_g = (float)raw_accel[1] / accel_sensitivity;
    data->accel_z_g = (float)raw_accel[2] / accel_sensitivity;

    // Convert raw gyroscope data to degrees per second.
    data->gyro_x_dps = (float)raw_gyro[0] / GYRO_SENS_250_DPS;
    data->gyro_y_dps = (float)raw_gyro[1] / GYRO_SENS_250_DPS;
    data->gyro_z_dps = (float)raw_gyro[2] / GYRO_SENS_250_DPS;

    // Convert raw temperature data to degrees Celsius using the formula from the datasheet.
    data->temperature_c = (raw_temp / 340.0f) + 36.53f;

    return true;
}

/**
 * @brief Gets the Wi-Fi connection status.
 */
int hal_wifi_get_status() {
    return cyw43_wifi_link_status(NULL, CYW43_ITF_STA);
}

/**
 * @brief CGI handler for POST requests.
 *
 * This function is called by the lwIP HTTP server when a request is made to a
 * registered CGI URI (in our case, "/command"). It re-assembles the parsed
 * key-value pairs into a single string and calls the user's callback.
 */
static const char* cgi_command_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    
    for(int i=0; i<iNumParams; i++) {
        printf("%s : %s\n", pcParam, pcValue);
    }

    return "/index.shtml";
}

// Array of CGI handlers. We only have one.
static const tCGI cgi_handlers[] = {
    {
        "/command",         // The URI that triggers this handler
        cgi_command_handler // The function to call
    }
};

/**
 * @brief Initializes the HTTP server.
 */
void hal_http_server_init() {
    cyw43_arch_lwip_begin();
    httpd_init();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
    printf("HAL: HTTP server initialized.\n");
    cyw43_arch_lwip_end();
}

/**
 * @brief Gets the IP address as a string.
 */
const char* hal_wifi_get_ip_address_str() {
    return ip4addr_ntoa(netif_ip4_addr(netif_list));
}