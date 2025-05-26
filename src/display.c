#include "display.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/auxdisplay.h>
// #include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(display);

#define DISPLAY_THREAD_STACK_SIZE 2048
#define DISPLAY_THREAD_PRIORITY 4

K_THREAD_STACK_DEFINE(display_stack, DISPLAY_THREAD_STACK_SIZE);

ZBUS_MSG_SUBSCRIBER_DEFINE(display_subscriber);
ZBUS_CHAN_ADD_OBS(display_chan, display_subscriber, 3);

static void display_thread(void *arg1, void *arg2, void *arg3) {
    int rc;
    const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(dummy_dc));

    if (!device_is_ready(dev)) {
        LOG_ERR("Auxdisplay device is not ready.");
    }

    rc = auxdisplay_cursor_set_enabled(dev, true);

    if (rc != 0) {
        LOG_ERR("Failed to enable cursor: %d", rc);
    }

    while (1) {
        struct display_data_t data;
        const struct zbus_channel *chan;
        if(!zbus_sub_wait_msg(&display_subscriber, &chan, &data, K_FOREVER)) {
            // rc = auxdisplay_write(dev, data.text, strlen(data.text));
            // if (rc != 0) {
            //     LOG_ERR("Failed to write data: %d", rc);
            // }
            LOG_INF("\t\t %s \t |brilho=%d, contraste=%d|", data.text, data.brightness, data.contrast);
        }
    }
}

K_THREAD_DEFINE(display_thread_id, DISPLAY_THREAD_STACK_SIZE, display_thread, NULL, NULL, NULL, DISPLAY_THREAD_PRIORITY, 0, 0);
ZBUS_CHAN_DEFINE(display_chan, struct display_data_t, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.brightness = 100, .contrast = 50, .text = "Hello, World!"));
