#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include "camera_lpr_thread.h"
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(camera_lpr);

// Definição dos canais ZBUS para trigger e resultado
ZBUS_CHAN_DEFINE(lpr_trigger_chan,         // nome do canal
                 int,                      // tipo de dado
                 NULL,                     // validator
                 NULL,                     // user_data
                 ZBUS_OBSERVERS_EMPTY,     // observers
                 ZBUS_MSG_INIT(0));        // valor inicial

struct lpr_result_t {
    char placa[16];
    char hash[32];
    bool valida;
};

ZBUS_CHAN_DEFINE(lpr_result_chan,
                 struct lpr_result_t,
                 NULL,
                 NULL,
                 ZBUS_OBSERVERS_EMPTY,
                 ZBUS_MSG_INIT(.placa = "", .hash = "", .valida = false));

#define LPR_STACK_SIZE 2048
#define LPR_PRIORITY 5

static void gerar_placa(char *placa, bool *valida) {
    uint32_t rand_val = sys_rand32_get();
    uint8_t fail_chance = rand_val % 100;
    if (fail_chance < CONFIG_RADAR_CAMERA_FAILURE_RATE_PERCENT) {
        // Placa inválida (falha simulada)
        placa[0] = '\0';
        *valida = false;
        LOG_INF("Falha simulada na câmera LPR");
        printk("Falha simulada na câmera LPR\n");
    } else {
        strcpy(placa, "BRA2E19"); // Exemplo Mercosul válida
        *valida = true;
    }
}

static void gerar_hash_foto(char *hash) {
    strcpy(hash, "HASH_PLACEHOLDER");
}

void camera_lpr_thread(void)
{
    while (1) {
        // Aguarda trigger via ZBUS
        int trigger;
        int ret = zbus_chan_read(&lpr_trigger_chan, &trigger, K_FOREVER);
        if (ret == 0 && trigger) {
            // Simula tempo de processamento caso habilitado
            if (CONFIG_SIMULATE_PROCESSING_TIME)
            {
                k_msleep(CONFIG_RADAR_PROCESSING_TIME_MS);
            }

            // Gera placa e hash
            char placa[16];
            char hash[32];
            bool valida;
            gerar_placa(placa, &valida);
            gerar_hash_foto(hash);

            struct {
                char placa[16];
                char hash[32];
                bool valida;
            } resultado;

            strcpy(resultado.placa, placa);
            strcpy(resultado.hash, hash);
            resultado.valida = valida;

            // Publica resultado no canal ZBUS
            zbus_chan_pub(&lpr_result_chan, &resultado, K_NO_WAIT);
        }
        // else: ignora triggers inválidos ou erros de leitura
    }
}

K_THREAD_DEFINE(camera_lpr_tid, LPR_STACK_SIZE, camera_lpr_thread, NULL, NULL, NULL, LPR_PRIORITY, 0, 0);
