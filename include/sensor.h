#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <stdint.h>

struct sensor_result {
    uint32_t timestamp_sensor1;
    uint32_t timestamp_sensor2;
    float velocidade_kmh;
};

void sensor_thread(void *arg1, void *arg2, void *arg3);

// Função de cálculo de velocidade (pode ser usada em testes)
float calcular_velocidade_kmh(uint32_t t1_ms, uint32_t t2_ms, float distancia_m);

#endif