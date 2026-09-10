#pragma once
#include <stdint.h>

extern uint32_t core_clock_counter;

int core_init();
void core_run();
void core_shutdown();

void core_advance_cpu_clocks(uint8_t clocks);