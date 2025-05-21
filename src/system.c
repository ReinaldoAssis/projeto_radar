#include "system.h"
#include "sensor.h"
// #include "camera_lpr_thread.h" // Remover, não é mais necessário
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
#define CAMERA_LPR_THREAD_STACK_SIZE 2048

#define DISPLAY_THREAD_PRIORITY 4
#define MAIN_THREAD_PRIORITY 5
#define NETWORK_THREAD_PRIORITY 4
#define CAMERA_LPR_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(display_stack, DISPLAY_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(main_stack, MAIN_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(network_stack, NETWORK_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(camera_lpr_stack, CAMERA_LPR_THREAD_STACK_SIZE);

struct k_thread display_thread_data;
struct k_thread main_thread_data;
struct k_thread network_thread_data;
struct k_thread camera_lpr_thread_data;

static void display_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        // print_log("Thread de Display executando...");
        k_msleep(1000);
    }
}

static void main_thread(void *arg1, void *arg2, void *arg3) {
    float velocidade;
    while (1) {
        int ret = zbus_chan_read(&velocidade_chan, &velocidade, K_FOREVER);
        if (ret == 0) {
            
            #ifdef CONFIG_RADAR_SPEED_LIMIT_KMH
            float limite = CONFIG_RADAR_SPEED_LIMIT_KMH;
            #else
            float limite = 60.0f;
            #endif

            if (velocidade > limite) {
                print_log("Infração detectada! Acionando câmera...");
                // Trigger via camera_service (canal ZBUS)
                int err = camera_api_capture(K_NO_WAIT);
                if (err) {
                    print_log("Falha ao acionar câmera via ZBUS!");
                }
            } else {
                // print_log("Velocidade dentro do limite.");
            }
        }
        k_msleep(100);
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

static void camera_lpr_thread_entry(void *arg1, void *arg2, void *arg3) {
    // camera_lpr_thread(); // Remover chamada antiga
    // Não faz mais nada, pois a lógica foi movida para camera_service
}

void init_threads(void)
{
    // k_thread_create(&display_thread_data, display_stack, DISPLAY_THREAD_STACK_SIZE,
    //                 display_thread, NULL, NULL, NULL,
    //                 DISPLAY_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&main_thread_data, main_stack, MAIN_THREAD_STACK_SIZE,
                    main_thread, NULL, NULL, NULL,
                    MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
    // k_thread_create(&network_thread_data, network_stack, NETWORK_THREAD_STACK_SIZE,
    //                 network_thread, NULL, NULL, NULL,
    //                 NETWORK_THREAD_PRIORITY, 0, K_NO_WAIT);
    // k_thread_create(&camera_lpr_thread_data, camera_lpr_stack, CAMERA_LPR_THREAD_STACK_SIZE,
    //                 camera_lpr_thread_entry, NULL, NULL, NULL,
    //                 CAMERA_LPR_THREAD_PRIORITY, 0, K_NO_WAIT);
    // Não cria mais a thread camera_lpr_thread
}

void sim_car_pass(void)
{
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
    init_threads();
    k_msleep(1000); // Aguarda um pouco para garantir que as threads estejam prontas
    print_log("Sistema inicializado!");

    if (CONFIG_SIM_CAR_PASSAGE) {
        sim_car_pass();
    }

    while (1) {
        k_msleep(1000);
    }
}
