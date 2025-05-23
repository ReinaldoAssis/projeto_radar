#ifndef CANAIS_H
#define CANAIS_H
#include <zephyr/zbus/zbus.h>

struct velocidade_evento_t {
    float velocidade_kmh;
    uint32_t event_id;
};

ZBUS_CHAN_DECLARE(velocidade_chan);

#endif
