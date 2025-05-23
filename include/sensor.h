#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zbus/zbus.h>

struct sensor_result {
    uint32_t timestamp_sensor1;
    uint32_t timestamp_sensor2;
    float velocidade_kmh;
};

// Torna sensor1 e sensor2 acessíveis globalmente
extern const struct gpio_dt_spec sensor1;
extern const struct gpio_dt_spec sensor2;

void sensor_thread(void *arg1, void *arg2, void *arg3);

// Função de cálculo de velocidade (pode ser usada em testes)
int calcular_velocidade_kmh(uint32_t t1_ms, uint32_t t2_ms, float distancia_m, float *velocidade_ptr);

struct velocidade_evento_t {
    float velocidade_kmh;
    uint32_t event_id;
};

struct sensor_evento_t {
    uint32_t timestamp_sensor1;
    uint32_t timestamp_sensor2;
};

ZBUS_CHAN_DECLARE(velocidade_chan);
ZBUS_CHAN_DECLARE(sensor_chan);

#endif