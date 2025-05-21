#include <zephyr/zbus/zbus.h>
#include "canais.h"

// Inicializa o canal com zeros
ZBUS_CHAN_DEFINE(velocidade_chan, struct velocidade_evento_t, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.velocidade_kmh = 0.0f, .event_id = 0));
