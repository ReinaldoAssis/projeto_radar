#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <zephyr/zbus/zbus.h>

struct display_data_t {
    int brightness;
    int contrast;
    char text[128];
};

ZBUS_CHAN_DECLARE(display_chan);

#endif