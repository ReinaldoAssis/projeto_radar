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

#define MAIN_THREAD_STACK_SIZE 1024
#define NETWORK_THREAD_STACK_SIZE 1024

#define MAIN_THREAD_PRIORITY 2
#define NETWORK_THREAD_PRIORITY 4

K_THREAD_STACK_DEFINE(main_stack, MAIN_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(network_stack, NETWORK_THREAD_STACK_SIZE);

static void network_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        // LOG_INF("Thread de Rede executando...");
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

    // Use o endereço do servidor NTP no formato string
    const char *ntp_server = "200.160.7.186:123";

    rc = sntp_simple(ntp_server, 10000, &ts);

    if (rc == 0) {
        LOG_INF("SNTP request successful.");
        LOG_INF("Time: %u seconds, %u fraction", ts.seconds, ts.fraction);

        time_t current_time = (time_t)ts.seconds;
        if (current_time >= 2208988800UL) {
            current_time -= 2208988800UL;
        } else {
            LOG_WRN("NTP time is before Unix epoch, cannot convert directly.");
        }

        char buffer[30];
        struct tm *tm_info = gmtime(&current_time);
        if (tm_info) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", tm_info);
            LOG_INF("Current UTC time: %s", buffer);
        } else {
            LOG_ERR("Failed to convert NTP time to human-readable format.");
        }

    } else {
        LOG_ERR("SNTP request failed, error: %d", rc);
    }
}
#endif

void sim_car_pass(void)
{
    // int random_delay = sys_rand32_get() % 300 + 100;
    int random_delay = 1;

    LOG_INF("Delay aleatório: %d ms", random_delay);

    LOG_INF("Carro passando...");
    gpio_emul_input_set(sensor1.port, sensor1.pin, 1);
    k_msleep(2); // Garante pulso detectável
    gpio_emul_input_set(sensor1.port, sensor1.pin, 0);

    k_msleep(random_delay); // Aguarda antes do segundo sensor

    gpio_emul_input_set(sensor2.port, sensor2.pin, 1);
    k_msleep(2); // Garante pulso detectável
    gpio_emul_input_set(sensor2.port, sensor2.pin, 0);

    LOG_INF("Carro passou!");
    k_msleep(300);
}

int main(void) {
    k_msleep(1000); // Aguarda um pouco para garantir que as threads estejam prontas
    LOG_INF("Sistema inicializado!");
    uint8_t sim_times = 0;

    while (1) {
        if (CONFIG_SIM_CAR_PASSAGE) {
            if (sim_times < 10) sim_car_pass();

            if (sim_times >= 10) {
                LOG_INF("=========================");
                LOG_INF("Simulação concluída.");
                LOG_INF("-------------------------");
                LOG_INF("Stats: ");

                break;
            }

            sim_times++;
        }
        #if CONFIG_TEST_SNTP
        test_sntp();
        #endif

        k_msleep(500);
    }
}
