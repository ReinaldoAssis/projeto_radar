#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sensor.h"

LOG_MODULE_REGISTER(main);

#define SENSOR_THREAD_STACK_SIZE 1024
#define DISPLAY_THREAD_STACK_SIZE 1024
#define MAIN_THREAD_STACK_SIZE 1024
#define NETWORK_THREAD_STACK_SIZE 1024

#define SENSOR_THREAD_PRIORITY 3
#define DISPLAY_THREAD_PRIORITY 4
#define MAIN_THREAD_PRIORITY 5
#define NETWORK_THREAD_PRIORITY 4

K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(display_stack, DISPLAY_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(main_stack, MAIN_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(network_stack, NETWORK_THREAD_STACK_SIZE);

struct k_thread sensor_thread_data;
struct k_thread display_thread_data;
struct k_thread main_thread_data;
struct k_thread network_thread_data;

void display_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        print_log("Thread de Display executando...");
        k_msleep(1000); // Simulação temporária
    }
}

void main_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        print_log("Thread Principal executando...");
        k_msleep(1000); // Simulação temporária
    }
}

void network_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        print_log("Thread de Rede executando...");
        k_msleep(1000); // Simulação temporária
    }
}

void print_log(const char *message) {
	LOG_INF("%s", message);
	printk("%s\n", message);
}

void main(void) {
    k_thread_create(&sensor_thread_data, sensor_stack, SENSOR_THREAD_STACK_SIZE,
                    sensor_thread, NULL, NULL, NULL,
                    SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&display_thread_data, display_stack, DISPLAY_THREAD_STACK_SIZE,
                    display_thread, NULL, NULL, NULL,
                    DISPLAY_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&main_thread_data, main_stack, MAIN_THREAD_STACK_SIZE,
                    main_thread, NULL, NULL, NULL,
                    MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&network_thread_data, network_stack, NETWORK_THREAD_STACK_SIZE,
                    network_thread, NULL, NULL, NULL,
                    NETWORK_THREAD_PRIORITY, 0, K_NO_WAIT);

	print_log("Sistema inicializado!");

	while (1) {
		k_msleep(1000); // Manter o loop principal ativo
	}
}