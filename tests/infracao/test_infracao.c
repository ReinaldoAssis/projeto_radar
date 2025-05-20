#include <zephyr/ztest.h>
#include <zephyr/zbus/zbus.h>
#include "camera_lpr_thread.h"

// Externs dos canais definidos no app
extern struct zbus_channel velocidade_chan;
extern struct zbus_channel lpr_trigger_chan;

// Helper para limpar o canal trigger
static void clear_lpr_trigger() {
    int dummy;
    // Limpa qualquer trigger pendente
    while (zbus_chan_read(&lpr_trigger_chan, &dummy, K_NO_WAIT) == 0) {}
}

ZTEST(integration, test_velocidade_sem_infracao)
{
    clear_lpr_trigger();
    float velocidade = 30.0f; // Supondo limite > 30
    zbus_chan_pub(&velocidade_chan, &velocidade, K_NO_WAIT);

    int trigger;
    // Espera um tempo razoável para garantir que não houve trigger
    int ret = zbus_chan_read(&lpr_trigger_chan, &trigger, K_MSEC(500));
    zassert_not_equal(ret, 0, "Trigger de câmera não deveria ocorrer para velocidade abaixo do limite");
}

ZTEST(integration, test_velocidade_com_infracao)
{
    clear_lpr_trigger();
    float velocidade = 100.0f; // Supondo limite < 100
    zbus_chan_pub(&velocidade_chan, &velocidade, K_NO_WAIT);

    int trigger;
    // Espera o trigger da câmera
    int ret = zbus_chan_read(&lpr_trigger_chan, &trigger, K_MSEC(500));
    zassert_equal(ret, 0, "Trigger de câmera deveria ocorrer para velocidade acima do limite");
    zassert_equal(trigger, 1, "Trigger de câmera deve ser 1");
}

ZTEST_SUITE(integration, NULL, NULL, NULL, NULL, NULL);