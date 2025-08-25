/**
 * @file hal.h
 * @brief Hardware Abstraction Layer for the robot platform.
 *
 * This file provides a high-level interface for interacting with the robot's
 * hardware components, including motors, the IMU (MPU6050), and the
 * onboard Wi-Fi module of the Pico W.
 */

#ifndef HAL_H
#define HAL_H

#include "pico/stdlib.h"

/**
 * @brief Structure to hold processed data from the IMU.
 *
 * All values are converted to standard physical units.
 */
typedef struct {
    float accel_x_g;    // Acceleration on X-axis in g's
    float accel_y_g;    // Acceleration on Y-axis in g's
    float accel_z_g;    // Acceleration on Z-axis in g's
    float gyro_x_dps;   // Gyroscopic speed on X-axis in degrees per second
    float gyro_y_dps;   // Gyroscopic speed on Y-axis in degrees per second
    float gyro_z_dps;   // Gyroscopic speed on Z-axis in degrees per second
    float temperature_c; // Temperature in degrees Celsius
} hal_imu_data_t;


/**
 * @brief Initializes all hardware components managed by the HAL.
 *
 * This function must be called once at the beginning of the application.
 * It sets up the standard I/O, motors, IMU, and initializes the Wi-Fi chip.
 * It also attempts to connect to the specified Wi-Fi network.
 *
 * @param wifi_ssid The SSID (name) of the Wi-Fi network to connect to.
 * @param wifi_password The password for the Wi-Fi network.
 * @return True if initialization was successful, false otherwise.
 */
bool hal_init(const char *wifi_ssid, const char *wifi_password);

/**
 * @brief Sets the state of the Pico W's onboard LED.
 *
 * @param on True to turn the LED on, false to turn it off.
 */
void hal_set_onboard_led(bool on);

/**
 * @brief Toggles the current state of the Pico W's onboard LED.
 */
void hal_toggle_onboard_led();

/**
 * @brief Sets the speed of the left and right motors.
 *
 * The speed is provided as a floating-point number from -1.0 to 1.0.
 * -  1.0: Full speed forward
 * -  0.0: Stop
 * - -1.0: Full speed reverse
 *
 * @param speed_left The desired speed for the left motor.
 * @param speed_right The desired speed for the right motor.
 */
void hal_motor_set_speeds(float speed_left, float speed_right);

/**
 * @brief Stops both motors immediately.
 *
 * This is a convenience function equivalent to calling hal_motor_set_speeds(0.0, 0.0).
 */
void hal_motor_stop();

/**
 * @brief Reads the latest data from the IMU.
 *
 * This function fetches the raw sensor data and converts it into
 * meaningful physical units, populating the provided data structure.
 *
 * @param data Pointer to a hal_imu_data_t struct to be filled with sensor data.
 * @return True on success, false on failure (though the current driver does not report errors).
 */
bool hal_imu_get_data(hal_imu_data_t *data);

/**
 * @brief Gets the current Wi-Fi connection status.
 *
 * Refer to the cyw43_wifi_link_status documentation for status codes.
 * Common values:
 * - CYW43_LINK_DOWN (0)
 * - CYW43_LINK_JOIN (1)
 * - CYW43_LINK_NOIP (2)
 * - CYW43_LINK_UP (3)
 * - CYW43_LINK_FAIL (-1)
 * - CYW43_LINK_NONET (-2)
 * - CYW43_LINK_BADAUTH (-3)
 *
 * @return The Wi-Fi link status code.
 */
int hal_wifi_get_status();

/**
 * @brief Initializes the onboard HTTP server.
 *
 * This function sets up a simple web server. It registers a handler for POST
 * requests to the "/command" URI, which will trigger the provided callback.
 *
 * @param post_callback A pointer to your function that will handle POST request data.
 */
void hal_http_server_init();

/**
 * @brief Gets the IP address assigned to the Pico W.
 *
 * @return A constant string representing the IP address (e.g., "192.168.1.10").
 * Returns "0.0.0.0" if not connected.
 */
const char* hal_wifi_get_ip_address_str();

#endif // HAL_H