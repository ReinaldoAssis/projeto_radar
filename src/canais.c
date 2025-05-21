#include <zephyr/zbus/zbus.h>

ZBUS_CHAN_DEFINE(velocidade_chan, float, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0.0f));
