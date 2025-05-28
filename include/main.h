#ifndef MAIN_H
#define MAIN_H

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <time.h>

enum network_event_type {
    NETWORK_EVENT_SNTP_RESPONSE,
    NETWORK_EVENT_HTTP_RESPONSE,
    NETWORK_EVENT_SNTP_REQUEST,
    NETWORK_EVENT_HTTP_REQUEST,
    NETWORK_EVENT_ERROR,
};

struct network_event_t {
    enum network_event_type type;
    union {
        struct {
            time_t unix_time;
            char time_str[49];
            int error_code;
        } sntp_response;

        struct {
            int status_code;
            char *response;
        } http_response;

        struct {
            char *request_url;
            int timeout_ms;
        } http_request;
    };
};

int get_converted_sntp_time(struct tm *tm_brazil, char *time_str);

ZBUS_CHAN_DECLARE(network_chan);

#endif