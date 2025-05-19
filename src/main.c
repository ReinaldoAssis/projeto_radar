#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

#define SENSOR_THREAD_STACK_SIZE 1024
#define DISPLAY_THREAD_STACK_SIZE 1024
#define MAIN_THREAD_STACK_SIZE 1024

#define SENSOR_THREAD_PRIORITY 3
#define DISPLAY_THREAD_PRIORITY 4
#define MAIN_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(display_stack, DISPLAY_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(main_stack, MAIN_THREAD_STACK_SIZE);

struct k_thread sensor_thread_data;
struct k_thread display_thread_data;
struct k_thread main_thread_data;

void sensor_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        LOG_INF("Thread de Sensores executando...");
        k_msleep(1000); // Simulação temporária
    }
}

void display_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        LOG_INF("Thread de Display executando...");
        k_msleep(1000); // Simulação temporária
    }
}

void main_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        LOG_INF("Thread Principal executando...");
        k_msleep(1000); // Simulação temporária
    }
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

    LOG_INF("Sistema inicializado!");
	printk("Sistema inicializado!\n");
	while (1) {
		k_msleep(1000); // Manter o loop principal ativo
	}
}