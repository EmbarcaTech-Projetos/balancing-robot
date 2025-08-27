#ifndef NETWORK_H_
#define NETWORK_H_

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "hal.h"

/**
 * @brief This function is called when a POST request begins.
 */
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd);

/**
 * @brief This function is called with chunks of POST data.
 */
err_t httpd_post_receive_data(void *connection, struct pbuf *p);

/**
 * @brief This function is called when the POST request is finished.
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len);

extern float kp, kd, ki;
extern float complementary_filter_alpha;
extern uint16_t deadzone;

#endif