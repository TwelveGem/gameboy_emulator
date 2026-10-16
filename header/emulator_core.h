#pragma once
#include "gfx.h"
#include <stdint.h>

extern gfx_context *gfx_debug_ctx;
extern gfx_context *gfx_render_ctx;
extern uint32_t core_clock_counter;

int core_init();
void core_run();
void core_shutdown();

void core_advance_cpu_clocks(uint8_t clocks);
