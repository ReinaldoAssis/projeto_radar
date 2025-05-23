#include "sensor.h"
#include "canais.h"
#include "camera_service.h" // Adicionar para acesso à API da câmera"

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <string.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(system);

#define DISPLAY_THREAD_STACK_SIZE 1024
#define MAIN_THREAD_STACK_SIZE 1024
#define NETWORK_THREAD_STACK_SIZE 1024

#define DISPLAY_THREAD_PRIORITY 4
#define MAIN_THREAD_PRIORITY 5
#define NETWORK_THREAD_PRIORITY 4

K_THREAD_STACK_DEFINE(display_stack, DISPLAY_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(main_stack, MAIN_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(network_stack, NETWORK_THREAD_STACK_SIZE);

struct k_thread display_thread_data;
struct k_thread main_thread_data;
struct k_thread network_thread_data;

static void display_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        // print_log("Thread de Display executando...");
        k_msleep(1000);
    }
}

static void network_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        // print_log("Thread de Rede executando...");
        k_msleep(1000);
    }
}

void print_log(const char *message) {
    LOG_INF("%s", message);
    printk("%s\n", message);
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
        print_log("SNTP request successful.");
        LOG_INF("Time: %u seconds, %u fraction", ts.seconds, ts.fraction);
        printk("Time: %u seconds, %u fraction\n", ts.seconds, ts.fraction);

        time_t current_time = (time_t)ts.seconds;
        if (current_time >= 2208988800UL) {
            current_time -= 2208988800UL;
        } else {
            print_log("Warning: NTP time is before Unix epoch, cannot convert directly.");
        }

        char buffer[30];
        struct tm *tm_info = gmtime(&current_time);
        if (tm_info) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", tm_info);
            LOG_INF("Current UTC time: %s", buffer);
            printk("Current UTC time: %s\n", buffer);
        } else {
            print_log("Failed to convert NTP time to human-readable format.");
        }

    } else {
        print_log("SNTP request failed.");
        LOG_ERR("SNTP error: %d", rc);
        printk("SNTP error: %d\n", rc);
    }
}
#endif

void sim_car_pass(void)
{
    int random_delay = sys_rand32_get() % 300 + 200;
    printk("Delay aleatório: %d ms\n", random_delay);

    print_log("Carro passando...");
    gpio_emul_input_set(sensor1.port, sensor1.pin, 1);
    k_msleep(2); // Garante pulso detectável
    gpio_emul_input_set(sensor1.port, sensor1.pin, 0);

    k_msleep(random_delay); // Aguarda antes do segundo sensor

    gpio_emul_input_set(sensor2.port, sensor2.pin, 1);
    k_msleep(2); // Garante pulso detectável
    gpio_emul_input_set(sensor2.port, sensor2.pin, 0);

    print_log("Carro passou!");
    k_msleep(300);
}

int main(void) {
    k_msleep(1000); // Aguarda um pouco para garantir que as threads estejam prontas
    print_log("Sistema inicializado!");

    while (1) {
        if (CONFIG_SIM_CAR_PASSAGE) {
            sim_car_pass();
        }
        #if CONFIG_TEST_SNTP
        test_sntp();
        #endif
        k_msleep(500);
    }
}