#include "sensor.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <inttypes.h>
#include <zephyr/zbus/zbus.h>
#include <stdio.h> // Incluído para usar printf

LOG_MODULE_REGISTER(sensor);

ZBUS_CHAN_DEFINE(velocidade_chan, struct velocidade_evento_t, NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.velocidade_kmh = 0.0f, .event_id = 0));

// Definição dos sensores como GPIOs (aliases sw0 e sw1)
#define SENSOR1_NODE DT_ALIAS(sw0)
#define SENSOR2_NODE DT_ALIAS(sw1)

#if !DT_NODE_EXISTS(SENSOR1_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
#if !DT_NODE_EXISTS(SENSOR2_NODE)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif

const struct gpio_dt_spec sensor1 = GPIO_DT_SPEC_GET(SENSOR1_NODE, gpios);
const struct gpio_dt_spec sensor2 = GPIO_DT_SPEC_GET(SENSOR2_NODE, gpios);

static struct gpio_callback sensor1_cb_data;
static struct gpio_callback sensor2_cb_data;

static volatile uint32_t timestamp_sensor1 = 0;
static volatile uint32_t timestamp_sensor2 = 0;
static volatile bool sensor1_activated = false;
static volatile bool sensor2_activated = false;

static void sensor1_triggered(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    if (!sensor1_activated) {
        timestamp_sensor1 = k_uptime_get_32();
        sensor1_activated = true;
        printk("Sensor 1 ativado em %" PRIu32 "\n", timestamp_sensor1);
    }
}

static void sensor2_triggered(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    if (!sensor2_activated && sensor1_activated) {
        timestamp_sensor2 = k_uptime_get_32();
        sensor2_activated = true;
        printk("Sensor 2 ativado em %" PRIu32 "\n", timestamp_sensor2);
    }
}

/**
 * @brief Calculate speed in km/h based on sensor triggers
 * 
 * @param t1_ms Timestamp of first sensor trigger in milliseconds
 * @param t2_ms Timestamp of second sensor trigger in milliseconds
 * @param distancia_m Distance between sensors in meters
 * @param velocidade_ptr Pointer to store the calculated speed value
 * @return int 0 on success, negative error code otherwise
 */
int calcular_velocidade_kmh(uint32_t t1_ms, uint32_t t2_ms, float distancia_m, float *velocidade_ptr) 
{
    if (velocidade_ptr == NULL) {
        return -EINVAL;
    }

    uint32_t dticks = t2_ms - t1_ms;
    float dt = (float)dticks / 1000.0f;

    if (dt <= 0.0f) {
        *velocidade_ptr = 0.0f;
        return -ERANGE;
    }

    float velocidade_ms = distancia_m / dt;

    if (velocidade_ms < 0.0f){
        *velocidade_ptr = 0.0f;
        return -ERANGE;
    }

    *velocidade_ptr = velocidade_ms * 3.6f;
    return 0;
}

#define SENSOR_THREAD_STACK_SIZE 1024
#define SENSOR_THREAD_PRIORITY 2

void sensor_thread(void *arg1, void *arg2, void *arg3) {
    int ret;
    
    if (!device_is_ready(sensor1.port) || !device_is_ready(sensor2.port)) {
        printk("Erro: GPIO dos sensores não está pronto\n");
        return;
    }
    
    ret = gpio_pin_configure_dt(&sensor1, GPIO_INPUT);
    if (ret != 0) {
        printk("Erro %d ao configurar sensor1\n", ret);
        return;
    }
    ret = gpio_pin_configure_dt(&sensor2, GPIO_INPUT);
    if (ret != 0) {
        printk("Erro %d ao configurar sensor2\n", ret);
        return;
    }
    
    ret = gpio_pin_interrupt_configure_dt(&sensor1, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        printk("Erro %d ao configurar interrupção sensor1\n", ret);
        return;
    }
    ret = gpio_pin_interrupt_configure_dt(&sensor2, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        printk("Erro %d ao configurar interrupção sensor2\n", ret);
        return;
    }
    
    gpio_init_callback(&sensor1_cb_data, sensor1_triggered, BIT(sensor1.pin));
    gpio_add_callback(sensor1.port, &sensor1_cb_data);
    gpio_init_callback(&sensor2_cb_data, sensor2_triggered, BIT(sensor2.pin));
    gpio_add_callback(sensor2.port, &sensor2_cb_data);

    printk("Thread de Sensores inicializada (aliases sw0 e sw1)\n");
    LOG_INF("Thread de Sensores inicializada (aliases sw0 e sw1)");

    static uint32_t event_id = 0;


    // TODO: não usar polling
    while (1) {
        if (sensor1_activated && sensor2_activated) {
            uint32_t dticks = timestamp_sensor2 - timestamp_sensor1;

            #ifdef CONFIG_RADAR_SENSOR_DISTANCE_MM
            float distancia_m = CONFIG_RADAR_SENSOR_DISTANCE_MM / 1000.0f;
            #else
            float distancia_m = 1.0f; // valor padrão 1 metro
            #endif
            
            float velocidade_kmh = 0.0f;
            int err = calcular_velocidade_kmh(timestamp_sensor1, timestamp_sensor2, distancia_m, &velocidade_kmh);
            // Use printf para imprimir float corretamente
            printf("Tempo: %u ms, Velocidade: %.2f km/h\n", dticks, velocidade_kmh);
            LOG_INF("Tempo: %u ms, Velocidade: %.2f km/h", dticks, velocidade_kmh);

            struct velocidade_evento_t evento = {
                .velocidade_kmh = velocidade_kmh,
                .event_id = ++event_id
            };
            zbus_chan_pub(&velocidade_chan, &evento, K_NO_WAIT);

            // Reset para próxima medição
            sensor1_activated = false;
            sensor2_activated = false;
        }
        k_msleep(100);
    }
}

// Criação automática da thread do sensor
K_THREAD_DEFINE(sensor_tid, SENSOR_THREAD_STACK_SIZE, sensor_thread, NULL, NULL, NULL, SENSOR_THREAD_PRIORITY, 0, 0);
