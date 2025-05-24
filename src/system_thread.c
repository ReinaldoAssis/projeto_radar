#include "camera_service.h"
#include "sensor.h"
#include "display.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
LOG_MODULE_REGISTER(system_thread);

#define SYSTEM_THREAD_STACK_SIZE 1024
#define SYSTEM_THREAD_PRIORITY 1


ZBUS_MSG_SUBSCRIBER_DEFINE(main_subscriber);
ZBUS_CHAN_ADD_OBS(velocidade_chan, main_subscriber, 3);

static bool validar_placa_mercosul(const char *placa, const char *padrao) {
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
        
        // retorna o tipo de padrão (LLLNLNN)
        if (padrao != NULL){
            strcpy(padrao, "LLLNLNN");
        }

        return true;
    }

    // Argentina/Paraguai: LLNNNLL (ex: AB123CD)
    if (len == 7 &&
        isalpha(placa_sem_espacos[0]) && isalpha(placa_sem_espacos[1]) &&
        isdigit(placa_sem_espacos[2]) && isdigit(placa_sem_espacos[3]) && isdigit(placa_sem_espacos[4]) &&
        isalpha(placa_sem_espacos[5]) && isalpha(placa_sem_espacos[6])) {

        // retorna o tipo de padrão (LLNNNLL)
        if (padrao != NULL){
            strcpy(padrao, "LLNNNLL");
        }
        return true;
    }

    // Argentina alternativa: LLLNNLL (ex: ABC12DE)
    if (len == 7 &&
        isalpha(placa_sem_espacos[0]) && isalpha(placa_sem_espacos[1]) && isalpha(placa_sem_espacos[2]) &&
        isdigit(placa_sem_espacos[3]) && isdigit(placa_sem_espacos[4]) &&
        isalpha(placa_sem_espacos[5]) && isalpha(placa_sem_espacos[6])) {
        
        // retorna o tipo de padrão (LLLNNLL)
        if (padrao != NULL){
            strcpy(padrao, "LLLNNLL");
        }
        return true;
    }

    return false;
}

static void system_thread(void *arg1, void *arg2, void *arg3) {
    struct velocidade_evento_t evento;
    const struct zbus_channel *chan;
    uint32_t last_event_id = 0;

    // Mensagem inicial para o display
    struct display_data_t display_msg = {
        .brightness = 100,
        .contrast = 50,
        .text = "Thread principal executando..."
    };
    zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

    LOG_INF("Thread principal executando...");

    // TODO [OK]: usar observer (message subscriber, zbus_sub_wait_msg)
    while (1) {
        if (!zbus_sub_wait_msg(&main_subscriber, &chan, &evento, K_FOREVER)) {
            const float limite = IS_ENABLED(CONFIG_RADAR_SPEED_LIMIT_KMH) ? CONFIG_RADAR_SPEED_LIMIT_KMH : 60.0f;

            if (evento.event_id != last_event_id && evento.velocidade_kmh > limite) {
                last_event_id = evento.event_id;

                // Mensagem de infração para o display
                snprintf(display_msg.text, sizeof(display_msg.text), "Infracao! Acionando camera...");
                zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                LOG_INF("\t- Infração detectada! Acionando câmera...");
                int err = camera_api_capture(K_NO_WAIT);
                if (err) {
                    LOG_ERR("Falha ao acionar câmera via ZBUS!");
                } else {
                    // Espera resposta da câmera
                    struct msg_camera_evt evt;
                    int evt_ret = zbus_chan_read(&chan_camera_evt, &evt, K_MSEC(500));
                    char padrao[8] = "";

                    if (evt_ret == 0) {
                        if (evt.type == MSG_CAMERA_EVT_TYPE_DATA && evt.captured_data) {
                            // Envia placa capturada para o display
                            snprintf(display_msg.text, sizeof(display_msg.text), "Placa: %s", evt.captured_data->plate);
                            zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                            // LOG_INF("\t- Camera: Placa capturada: %s", evt.captured_data->plate);

                            if (validar_placa_mercosul(evt.captured_data->plate, padrao)) {
                                snprintf(display_msg.text, sizeof(display_msg.text), "Placa valida (%s)", padrao);
                                zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                                // LOG_INF("\t- Placa válida (%s)", padrao);
                            } else {
                                snprintf(display_msg.text, sizeof(display_msg.text), "Placa invalida!");
                                zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                                LOG_WRN("\t- Placa inválida!");
                            }
                        } else if (evt.type == MSG_CAMERA_EVT_TYPE_ERROR) {
                            snprintf(display_msg.text, sizeof(display_msg.text), "Camera erro: %d", evt.error_code);
                            zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                            LOG_ERR("\t- Camera: Erro ao capturar imagem, code: %d", evt.error_code);
                        } else {
                            snprintf(display_msg.text, sizeof(display_msg.text), "Camera: Evento desconhecido");
                            zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                            LOG_WRN("\t- Camera: Evento desconhecido");
                        }
                    } else {
                        snprintf(display_msg.text, sizeof(display_msg.text), "Camera: Sem resposta");
                        zbus_chan_pub(&display_chan, &display_msg, K_NO_WAIT);

                        LOG_WRN("\t- Camera: Nenhuma resposta recebida");
                    }
                }
            }
        }
    }
}

K_THREAD_DEFINE(system_thread_id, SYSTEM_THREAD_STACK_SIZE, system_thread, NULL, NULL, NULL, SYSTEM_THREAD_PRIORITY, 0, 0);
