alias b := build
alias c := clean
alias f := flash
alias r := run
alias m := monitor
alias t := test
alias br := build_run
alias bt := run_all_tests
alias active := activate

default:
    just --list

clean:
    rip build

build:
    west build -p -b qemu_cortex_m3 .

activate:
    source ~/zephyrproject/.venv/bin/activate

build_run:
    west build --pristine -b qemu_cortex_m3 .
    west build -t run

test:
    rm -rf twister-out
    west twister -p qemu_cortex_m3 -T tests

run_sensor_tests:
    rm -rf twister-out
    west twister -p qemu_cortex_m3 -T tests/sensor

run_validar_placa_tests:
    rm -rf twister-out
    west twister -p qemu_cortex_m3 -T tests/validar_placa -Wno-unused-function

run_all_tests:
    rm -rf twister-out
    just run_validar_placa_tests
    just run_sensor_tests

flash:
    west flash

run:
    west build -t run

monitor:
    tio -b 115200 /dev/tty.usbmodem0006831335301

