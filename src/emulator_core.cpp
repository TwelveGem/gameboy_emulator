#include "emulator_core.h"
#include "cart.h"
#include "cpu.h"
#include "timer.h"

uint32_t core_clock_counter = 0;
bool core_quit_requested = false;

int core_init() {
    const char *game_path = "./games/rhythm-land.gb";

    if (!cart_load(game_path)) {
        return -1;
    }

    cart_print_info();

    return 0;
}

void core_run() {
    cpu_reset();

    while (!core_quit_requested) {
        cpu_tick();
    }
}

void core_shutdown() {}

void core_advance_cpu_clocks(uint8_t clocks) {
    timer_advance_clocks(clocks);

    core_clock_counter += clocks;
}