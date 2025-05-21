#include "system.h"
#include "sensor.h"
#include "canais.h"
#include "camera_service.h" // Adicionar para acesso à API da câmera"

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <string.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

extern struct zbus_channel velocidade_chan;

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


void sim_car_pass(void)
{

    int random_delay = sys_rand32_get() % 1000; // Gera um atraso aleatório entre 0 e 999 ms

    // Simula a passagem de um carro
    print_log("Carro passando...");
    gpio_emul_input_set(sensor1.port, sensor1.pin, 1);
    k_msleep(100); // Simula o tempo de passagem
    gpio_emul_input_set(sensor1.port, sensor1.pin, 0);
    k_msleep(100); // Aguarda um pouco antes de ativar o segundo sensor
    gpio_emul_input_set(sensor2.port, sensor2.pin, 1);
    k_msleep(100); // Simula o tempo de passagem
    gpio_emul_input_set(sensor2.port, sensor2.pin, 0);
    print_log("Carro passou!");
    // Aguarda um tempo para simular o processamento
    k_msleep(1000);

    print_log("Carro passando beeem rápiddoooo vrummmmm...");
    gpio_emul_input_set(sensor1.port, sensor1.pin, 1);
    k_msleep(10); // Simula o tempo de passagem
    gpio_emul_input_set(sensor1.port, sensor1.pin, 0);
    k_msleep(10); // Aguarda um pouco antes de ativar o segundo sensor
    gpio_emul_input_set(sensor2.port, sensor2.pin, 1);
    k_msleep(10); // Simula o tempo de passagem
    gpio_emul_input_set(sensor2.port, sensor2.pin, 0);
    print_log("Carro passou! Eita caraaaai boommm!");
    k_msleep(1000);
}

void system_start(void)
{
    k_msleep(1000); // Aguarda um pouco para garantir que as threads estejam prontas
    print_log("Sistema inicializado!");

    while (1) {
        if (CONFIG_SIM_CAR_PASSAGE) {
            sim_car_pass();
        }
        k_msleep(8000);
    }
}
