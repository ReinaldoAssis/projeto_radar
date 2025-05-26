cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(validar_placa_test)

zephyr_include_directories(../../include/)

target_sources(app PRIVATE ${app_sources} ../../src/system_thread.c ./test_validacao.c)
