#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../include/sensor.h"

/**
 * @brief Teste básico de cálculo de velocidade.
 *
 * Verifica o cálculo de velocidade com valores típicos de tempo e distância.
 */
ZTEST(sensor, test_velocidade_basica) {
    float distancia_m = 2.0f; /* 2 metros entre sensores */
    uint32_t t1 = 1000; /* ms */
    uint32_t t2 = 2000; /* ms */
    float v = calcular_velocidade_kmh(t1, t2, distancia_m);
    zassert_within(v, 7.2f, 0.01f, "Velocidade deve ser 7.2 km/h");
}

/**
 * @brief Teste de cálculo de velocidade com tempo curto.
 *
 * Verifica o cálculo de velocidade quando o intervalo de tempo entre
 * as medições é pequeno, resultando em uma velocidade maior.
 */
ZTEST(sensor, test_velocidade_rapida) {
    float distancia_m = 1.0f;
    uint32_t t1 = 1000;
    uint32_t t2 = 1010; /* 100 ms */
    float v = calcular_velocidade_kmh(t1, t2, distancia_m);
    zassert_within(v, 360.0f, 0.01f, "Velocidade deve ser 360.0 km/h");
}

/**
 * @brief Teste de cálculo de velocidade com tempo zero.
 *
 * Verifica o comportamento da função quando o tempo de passagem é zero
 * (t1 == t2), o que deve resultar em velocidade zero.
 */
ZTEST(sensor, test_velocidade_zero) {
    float distancia_m = 1.0f;
    uint32_t t1 = 1000;
    uint32_t t2 = 1000; /* sem passagem */
    float v = calcular_velocidade_kmh(t1, t2, distancia_m);
    zassert_equal(v, 0.0f, "Velocidade deve ser 0.0 km/h");
}

/**
 * @brief Teste de cálculo de velocidade com distância zero.
 *
 * Verifica o comportamento da função quando a distância entre os sensores
 * é zero, o que deve resultar em velocidade zero.
 */
ZTEST(sensor, test_dist_zero) {
    float distancia_m = 0.0f;
    uint32_t t1 = 1000;
    uint32_t t2 = 2000; /* ms */
    float v = calcular_velocidade_kmh(t1, t2, distancia_m);
    zassert_equal(v, 0.0f, "Velocidade deve ser 0.0 km/h");
}

/**
 * @brief Teste de cálculo de velocidade com distância negativa.
 *
 * Verifica o comportamento da função quando a distância entre os sensores
 * é negativa. A função deve tratar isso como um erro ou - nesse caso -
 * retornar zero.
 */
ZTEST(sensor, test_dist_negativa) {
    float distancia_m = -1.0f;
    uint32_t t1 = 1000;
    uint32_t t2 = 2000; /* ms */
    float v = calcular_velocidade_kmh(t1, t2, distancia_m);
    zassert_equal(v, 0.0f, "Velocidade deve ser 0.0 km/h");
}

ZTEST_SUITE(sensor, NULL, NULL, NULL, NULL, NULL);

