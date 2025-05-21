#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include "canais.h"
#include "camera_service.h"
LOG_MODULE_REGISTER(main_thread);

extern struct zbus_channel velocidade_chan;

#define MAIN_THREAD_STACK_SIZE 1024
#define MAIN_THREAD_PRIORITY 1

static void main_thread(void *arg1, void *arg2, void *arg3) {
    float velocidade;
    bool triggered = false; // Latch para evitar múltiplos triggers

    LOG_INF("Thread principal executando...");
    printf("Thread principal executando...\n");
    while (1) {
        int ret = zbus_chan_read(&velocidade_chan, &velocidade, K_FOREVER);
        if (ret == 0) {
            #ifdef CONFIG_RADAR_SPEED_LIMIT_KMH
            float limite = CONFIG_RADAR_SPEED_LIMIT_KMH;
            #else
            float limite = 60.0f;
            #endif

            if (velocidade > limite) {
                if (!triggered) {
                    LOG_INF("Infração detectada! Acionando câmera...");
                    printf("Infração detectada! Acionando câmera...\n");
                    int err = camera_api_capture(K_NO_WAIT);
                    if (err) {
                        LOG_ERR("Falha ao acionar câmera via ZBUS!");
                        printf("Falha ao acionar câmera via ZBUS!\n");
                    } else {
                        // Espera resposta da câmera
                        struct msg_camera_evt evt;
                        int evt_ret = zbus_chan_read(&chan_camera_evt, &evt, K_MSEC(500));
                        if (evt_ret == 0) {
                            if (evt.type == MSG_CAMERA_EVT_TYPE_DATA && evt.captured_data) {
                                printf("Camera: Placa capturada: %s\n", evt.captured_data->plate);
                                LOG_INF("Camera: Placa capturada: %s", evt.captured_data->plate);
                            } else if (evt.type == MSG_CAMERA_EVT_TYPE_ERROR) {
                                printf("Camera: Erro ao capturar imagem, code: %d\n", evt.error_code);
                                LOG_ERR("Camera: Erro ao capturar imagem, code: %d", evt.error_code);
                            } else {
                                printf("Camera: Evento desconhecido\n");
                                LOG_WRN("Camera: Evento desconhecido");
                            }
                        } else {
                            printf("Camera: Nenhuma resposta recebida\n");
                            LOG_WRN("Camera: Nenhuma resposta recebida");
                        }
                    }
                    triggered = true; // Dispara apenas uma vez
                }
            } else {
                triggered = false; // Libera para novo trigger quando voltar ao normal
            }
        }
        k_msleep(100);
    }
}

K_THREAD_DEFINE(main_thread_id, MAIN_THREAD_STACK_SIZE, main_thread, NULL, NULL, NULL, MAIN_THREAD_PRIORITY, 0, 0);
