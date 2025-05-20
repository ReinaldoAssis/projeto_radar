#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include "sensor.h"
#include "camera_lpr_thread.h"

extern struct zbus_channel lpr_trigger_chan;

LOG_MODULE_REGISTER(main);

#define SENSOR_THREAD_STACK_SIZE 1024
#define DISPLAY_THREAD_STACK_SIZE 1024
#define MAIN_THREAD_STACK_SIZE 1024
#define NETWORK_THREAD_STACK_SIZE 1024
#define CAMERA_LPR_THREAD_STACK_SIZE 2048

#define SENSOR_THREAD_PRIORITY 3
#define DISPLAY_THREAD_PRIORITY 4
#define MAIN_THREAD_PRIORITY 5
#define NETWORK_THREAD_PRIORITY 4
#define CAMERA_LPR_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(display_stack, DISPLAY_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(main_stack, MAIN_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(network_stack, NETWORK_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(camera_lpr_stack, CAMERA_LPR_THREAD_STACK_SIZE);

struct k_thread sensor_thread_data;
struct k_thread display_thread_data;
struct k_thread main_thread_data;
struct k_thread network_thread_data;
struct k_thread camera_lpr_thread_data;

// Canal ZBUS para comunicação de velocidade
ZBUS_CHAN_DEFINE(velocidade_chan, float, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0.0f));

void display_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        print_log("Thread de Display executando...");
        k_msleep(1000); // Simulação temporária
    }
}

void main_thread(void *arg1, void *arg2, void *arg3) {
    float velocidade;
    while (1) {
        // Aguarda velocidade dos sensores
        int ret = zbus_chan_read(&velocidade_chan, &velocidade, K_FOREVER);
        if (ret == 0) {
            print_log("Thread Principal: velocidade recebida");
            // Detecção de infração
			
			#ifdef CONFIG_RADAR_SPEED_LIMIT_KMH
            float limite = CONFIG_RADAR_SPEED_LIMIT_KMH;
			#else
            float limite = 60.0f; // valor padrão
			#endif

            if (velocidade > limite) {
                print_log("Infração detectada! Acionando câmera...");
                int trigger = 1;
                zbus_chan_pub(&lpr_trigger_chan, &trigger, K_NO_WAIT);
            } else {
                print_log("Velocidade dentro do limite.");
            }
        }

        k_msleep(100); // evitar busy loop
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

void camera_lpr_thread_entry(void *arg1, void *arg2, void *arg3) {
    // Placeholder: chamada para a função real da thread LPR
    camera_lpr_thread();
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
    k_thread_create(&camera_lpr_thread_data, camera_lpr_stack, CAMERA_LPR_THREAD_STACK_SIZE,
                    camera_lpr_thread, NULL, NULL, NULL,
                    CAMERA_LPR_THREAD_PRIORITY, 0, K_NO_WAIT);
	

	print_log("Sistema inicializado!");

	while (1) {
		k_msleep(1000); // Manter o loop principal ativo
	}
}