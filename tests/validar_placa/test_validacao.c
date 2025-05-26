#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../include/system_thread.h"

/*
Argentina
LL NNN LL

Brasil
LLL N L NN

Paraguai
LLLL NNN - Carro
NNN LLLL - Moto

Uruguai
LLL NNNN

Bolivia
LL NNNNN
*/

/**
 * @brief 
 */
ZTEST(validar, test_validar_placa_brasil) {
    char *placa = "ABC1D23";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_true(resultado, "Placa do Brasil inválida");
    zassert_str_equal(padrao, PLACA_BRASIL, "Padrão de placa do Brasil inválido");
}

/**
 * @brief 
 */
ZTEST(validar, test_validar_placa_argentina) {
    char *placa = "AB123CD";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_true(resultado, "Placa da Argentina inválida");
    zassert_str_equal(padrao, PLACA_ARGENTINA, "Padrão de placa da Argentina inválido");
}

/**
 * @brief 
 */
ZTEST(validar, test_validar_placa_paraguai_carro) {
    char *placa = "ABCD123";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_true(resultado, "Placa do Paraguai (Carro) inválida");
    zassert_str_equal(padrao, PLACA_PARAGUAI_CARRO, "Padrão de placa do Paraguai (Carro) inválido");
}

/**
 * @brief 
 */
ZTEST(validar, test_validar_placa_paraguai_moto) {
    char *placa = "123ABCD";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_true(resultado, "Placa do Paraguai (Moto) inválida");
    zassert_str_equal(padrao, PLACA_PARAGUAI_MOTO, "Padrão de placa do Paraguai (Moto) inválido");
}
/**
 * @brief 
 */
ZTEST(validar, test_validar_placa_uruguai) {
    char *placa = "ABC1234";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_true(resultado, "Placa do Uruguai inválida");
    zassert_str_equal(padrao, PLACA_URUGUAI, "Padrão de placa do Uruguai inválido");
}
/**
 * @brief 
 */
ZTEST(validar, test_validar_placa_bolivia) {
    char *placa = "AB12345";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_true(resultado, "Placa da Bolívia inválida");
    zassert_str_equal(padrao, PLACA_BOLIVIA, "Padrão de placa da Bolívia inválido");
}

/**
 * @brief Teste de placa inválida
 */
ZTEST(validar, test_validar_placa_invalida) {
    char *placa = "INVALIDA";
    char padrao[64] = "";
    bool resultado = validar_placa_mercosul(placa, padrao);
    zassert_false(resultado, "Placa inválida não deveria ser aceita");
}


ZTEST_SUITE(validar, NULL, NULL, NULL, NULL, NULL);

