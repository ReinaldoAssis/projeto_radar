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

static void network_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_msleep(1000);
    }
}

#if CONFIG_TEST_SNTP
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>

void test_sntp(void)
{
    struct sntp_time ts;
    int rc;    
    const char *ntp_server = "pool.ntp.org:123";

    rc = sntp_simple(ntp_server, 10000, &ts);    if (rc == 0) {
        
        time_t unix_time;
        
        if (ts.seconds < NTP_TIMESTAMP_DIFF) {

            unix_time = (time_t)ts.seconds;
        } else {
            unix_time = (time_t)(ts.seconds - NTP_TIMESTAMP_DIFF);
        }       
        time_t datetime = unix_time + TIMEZONE_OFFSET_SECONDS;

        struct tm *tm_brazil = gmtime(&datetime);
        if (tm_brazil) {
            char time_str[49];
            strftime(time_str, 48, "[%Y-%m-%d %H:%M:%S]", tm_brazil);
            LOG_INF("Converted time: %s", time_str);
        } else {
            LOG_ERR("Failed to convert time to struct tm");
        }

    } else {
        LOG_ERR("SNTP request failed, error: %d", rc);
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
