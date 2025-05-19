alias b := build
alias c := clean
alias f := flash
alias r := run
alias m := monitor
alias t := test
alias br := build_run
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
    west twister -p qemu_cortex_m3 -T tests

run_button_tests: && run
    west build -p -b qemu_cortex_m3 ./tests/button

flash:
    west flash

run:
    west build -t run

monitor:
    tio -b 115200 /dev/tty.usbmodem0006831335301
