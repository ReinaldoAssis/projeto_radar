#include "main.h"
#include "sensor.h"
#include "camera_service.h"
#include "display.h"

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <string.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/random/random.h>
#include <zephyr/drivers/auxdisplay.h>
#include <zephyr/device.h>

#ifdef CONFIG_SNTP
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#endif

LOG_MODULE_REGISTER(system);

#define SYSTEM_THREAD_STACK_SIZE 1024
#define NETWORK_THREAD_STACK_SIZE 1024

#define SYSTEM_THREAD_PRIORITY 2
#define NETWORK_THREAD_PRIORITY 4

K_THREAD_STACK_DEFINE(system_stack, SYSTEM_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(network_stack, NETWORK_THREAD_STACK_SIZE);

/*
    NTP to UTC Implementation by Matt
    at https://stackoverflow.com/questions/5206857/convert-ntp-timestamp-to-utc

    NTP_TIMESTAMP_DIFF - The difference in seconds between the NTP epoch (1900) and the Unix epoch (1970).
    NTP_MAX_INT_AS_DOUBLE - The maximum value of the fractional part of an NTP timestamp, which is 2^32 - 1.
*/
#define NTP_TIMESTAMP_DIFF     (2208988800)
#define NTP_MAX_INT_AS_DOUBLE  (4294967295.0)

#define TIMEZONE_OFFSET_SECONDS (-3 * 3600)

ZBUS_MSG_SUBSCRIBER_DEFINE(network_subscriber);
ZBUS_CHAN_ADD_OBS(network_chan, network_subscriber, 3);

/*
    @brief Converts SNTP time to a struct tm in Brazil's timezone and formats it as a string.
    @param tm_brazil Pointer to a struct tm to store the converted time.
    @param time_str Pointer to a string buffer to store the formatted time.
    @return 0 on success, negative error code on failure.
*/
int get_converted_sntp_time(struct tm *tm_brazil, char *time_str)
{
    struct sntp_time ts;
    int rc;
#ifdef CONFIG_SNTP_SERVER_ADDRESS
    const char *ntp_server = CONFIG_SNTP_SERVER_ADDRESS;
#else
    const char *ntp_server = "pool.ntp.org:123";
#endif

#ifdef CONFIG_SNTP_SERVER_TIMEOUT_MS
    const int timeout_ms = CONFIG_SNTP_SERVER_TIMEOUT_MS;
#else
    const int timeout_ms = 5000; 
#endif


    rc = sntp_simple(ntp_server, timeout_ms, &ts);
    if (rc == 0) {

        time_t unix_time;

        if (ts.seconds < NTP_TIMESTAMP_DIFF) {

            unix_time = (time_t)ts.seconds;
        } else {
            unix_time = (time_t)(ts.seconds - NTP_TIMESTAMP_DIFF);
        }       
        time_t datetime = unix_time + TIMEZONE_OFFSET_SECONDS;

        struct tm *_tm_brazil = gmtime(&datetime);
        if (_tm_brazil) {
            char _time_str[49];
            strftime(_time_str, 48, "[%Y-%m-%d %H:%M:%S]", _tm_brazil);

            if (time_str != NULL) {
                strcpy(time_str, _time_str);
                time_str[48] = '\0';
            }
            
            if (tm_brazil != NULL) {
                *tm_brazil = *_tm_brazil;
            }
            

            return 0;

        } else {
            return -1;
        }

    } else {
        return rc;
    }
}

static void network_thread(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (1) {
        struct network_event_t data;
        const struct zbus_channel *chan;

        /*
            When the system thread identifies a violation, it will publish a request
            in the network channel.
        */
        if (!zbus_sub_wait_msg(&network_subscriber, &chan, &data, K_FOREVER)) {
            switch (data.type) {
                case NETWORK_EVENT_SNTP_REQUEST: {
                    struct tm tm_brazil;
                    char time_str[49];
                    int ret = get_converted_sntp_time(&tm_brazil, time_str);

                    data = (struct network_event_t) {
                        .type = NETWORK_EVENT_SNTP_RESPONSE,
                        .sntp_response = {
                            .unix_time = mktime(&tm_brazil),
                            .time_str = time_str,
                            .error_code = ret
                        }
                    };

                    int err = zbus_chan_pub(&network_chan, &data, K_NO_WAIT);

                    if (err) {
                        LOG_ERR("Failed to publish SNTP response: %d", err);
                    }

                    break;
                }
                
                default:
                    LOG_WRN("Unknown network event type: %d", data.type);
                    break;
            }
        }
    }
}

#if CONFIG_TEST_SNTP

void test_sntp(void)
{

    char time_str[49];
    int ret = get_converted_sntp_time(NULL, time_str);

    if (ret == 0) {
        LOG_INF("SNTP Converted Time: %s", time_str);
    } else {
        LOG_ERR("Failed to get SNTP time: %d", ret);
    }
    
}
#endif

void sim_car_pass(void)
{
    int random_delay = sys_rand32_get() % 300 + 100;

    LOG_INF("Delay aleatório: %d ms", random_delay);

    LOG_INF("Carro passando...");
    gpio_emul_input_set(sensor1.port, sensor1.pin, 1);
    k_msleep(2);
    gpio_emul_input_set(sensor1.port, sensor1.pin, 0);

    k_msleep(random_delay);

    gpio_emul_input_set(sensor2.port, sensor2.pin, 1);
    k_msleep(2);
    gpio_emul_input_set(sensor2.port, sensor2.pin, 0);

    LOG_INF("Carro passou!");
    k_msleep(300);
}

int main(void) {

    /*
    Waits for the system to be ready before starting simulation.
    */
    k_msleep(1000);

    LOG_INF("Sistema inicializado!");
    uint8_t sim_times = 5;

    while (1) {
#ifdef CONFIG_SIM_CAR_PASSAGE
        if (CONFIG_SIM_CAR_PASSAGE) {
            if (sim_times > 0) sim_car_pass();

            if (sim_times == 0) {
                break;
            }

            sim_times--;
        }
#endif
        #if CONFIG_TEST_SNTP
        test_sntp();
        #endif

        k_msleep(500);
    }
}

ZBUS_CHAN_DEFINE(network_chan, struct network_event_t, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.type = NETWORK_EVENT_SNTP_RESPONSE, .sntp_response = {.unix_time = 0, .error_code = 0}));
K_THREAD_DEFINE(network_thread_id, SYSTEM_THREAD_STACK_SIZE, network_thread, NULL, NULL, NULL, SYSTEM_THREAD_PRIORITY, 0, 0);
