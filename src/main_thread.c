#include "camera_service.h"
#include "sensor.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
LOG_MODULE_REGISTER(main_thread);

#define MAIN_THREAD_STACK_SIZE 1024
#define MAIN_THREAD_PRIORITY 1

static bool validar_placa_mercosul(const char *placa) {
    if (!placa) return false;

    // Remove espaços da string (cria uma cópia sem espaços)
    char placa_sem_espacos[16]; // tamanho suficiente para placa + \0
    int j = 0;
    for (int i = 0; placa[i] != '\0' && j < (int)(sizeof(placa_sem_espacos) - 1); i++) {
        if (placa[i] != ' ')
            placa_sem_espacos[j++] = placa[i];
    }
    placa_sem_espacos[j] = '\0';

    size_t len = strlen(placa_sem_espacos);

    // Brasil: LLLNLNN (ex: BRA1B23)
    if (len == 7 &&
        isalpha(placa_sem_espacos[0]) && isalpha(placa_sem_espacos[1]) && isalpha(placa_sem_espacos[2]) &&
        isdigit(placa_sem_espacos[3]) &&
        isalpha(placa_sem_espacos[4]) &&
        isdigit(placa_sem_espacos[5]) && isdigit(placa_sem_espacos[6])) {
        return true;
    }

    // Argentina/Paraguai: LLNNNLL (ex: AB123CD)
    if (len == 7 &&
        isalpha(placa_sem_espacos[0]) && isalpha(placa_sem_espacos[1]) &&
        isdigit(placa_sem_espacos[2]) && isdigit(placa_sem_espacos[3]) && isdigit(placa_sem_espacos[4]) &&
        isalpha(placa_sem_espacos[5]) && isalpha(placa_sem_espacos[6])) {
        return true;
    }

    // Argentina alternativa: LLLNNLL (ex: ABC12DE)
    if (len == 7 &&
        isalpha(placa_sem_espacos[0]) && isalpha(placa_sem_espacos[1]) && isalpha(placa_sem_espacos[2]) &&
        isdigit(placa_sem_espacos[3]) && isdigit(placa_sem_espacos[4]) &&
        isalpha(placa_sem_espacos[5]) && isalpha(placa_sem_espacos[6])) {
        return true;
    }

    return false;
}

static void main_thread(void *arg1, void *arg2, void *arg3) {
    struct velocidade_evento_t evento;
    uint32_t last_event_id = 0;

    LOG_INF("Thread principal executando...");
    printf("Thread principal executando...\n");
    // TODO: usar observer (message subscriber, zbus_sub_wait_msg)
    while (1) {
        int ret = zbus_chan_read(&velocidade_chan, &evento, K_FOREVER);
        if (ret == 0) {
            const float limite = IS_ENABLED(CONFIG_RADAR_SPEED_LIMIT_KMH) ? CONFIG_RADAR_SPEED_LIMIT_KMH : 60.0f;

            if (evento.event_id != last_event_id && evento.velocidade_kmh > limite) {
                last_event_id = evento.event_id;
                LOG_INF("\t- Infração detectada! Acionando câmera...");
                printf("\t- Infração detectada! Acionando câmera...\n");
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
                            printf("\t- Camera: Placa capturada: %s\n", evt.captured_data->plate);
                            LOG_INF("\t- Camera: Placa capturada: %s", evt.captured_data->plate);
                            if (validar_placa_mercosul(evt.captured_data->plate)) {
                                printf("\t- Placa válida (Mercosul)\n");
                                LOG_INF("\t- Placa válida (Mercosul)");
                            } else {
                                printf("\t- Placa inválida!\n");
                                LOG_WRN("\t- Placa inválida!");
                            }
                        } else if (evt.type == MSG_CAMERA_EVT_TYPE_ERROR) {
                            printf("\t- Camera: Erro ao capturar imagem, code: %d\n", evt.error_code);
                            LOG_ERR("\t- Camera: Erro ao capturar imagem, code: %d", evt.error_code);
                        } else {
                            printf("\t- Camera: Evento desconhecido\n");
                            LOG_WRN("\t- Camera: Evento desconhecido");
                        }
                    } else {
                        printf("\t- Camera: Nenhuma resposta recebida\n");
                        LOG_WRN("\t- Camera: Nenhuma resposta recebida");
                    }
                }
            }
        }
        k_msleep(100);
    }
}

K_THREAD_DEFINE(main_thread_id, MAIN_THREAD_STACK_SIZE, main_thread, NULL, NULL, NULL, MAIN_THREAD_PRIORITY, 0, 0);
