# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/camera_service/")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(projeto_radar)

zephyr_include_directories(include/)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_sources(app PRIVATE src/main.c src/display.c src/sensor.c src/system_thread.c)

