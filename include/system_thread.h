#ifndef SYSTEM_THREAD_H
#define SYSTEM_THREAD_H

#define PLACA_BRASIL        "Brasil [LLL N L NN]"
#define PLACA_ARGENTINA     "Argentina [LL NNN LL]"
#define PLACA_PARAGUAI_CARRO "Paraguai (Carro) [LLLL NNN]"
#define PLACA_PARAGUAI_MOTO  "Paraguai (Moto) [NNN LLLL]"
#define PLACA_URUGUAI       "Uruguai [LLL NNNN]"
#define PLACA_BOLIVIA       "Bolivia [LL NNNNN]"

static void system_thread(void *arg1, void *arg2, void *arg3);
bool validar_placa_mercosul(const char *placa, char *padrao);

#endif // SYSTEM_THREAD_H