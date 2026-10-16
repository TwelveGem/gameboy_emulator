#include "emulator_core.h"
#include "cart.h"
#include "cpu.h"
#include "gfx.h"
#include "ppu.h"
#include "timer.h"

gfx_context *gfx_debug_ctx = nullptr;
gfx_context *gfx_render_ctx = nullptr;

uint32_t core_clock_counter = 0;
bool core_quit_requested = false;

void core_init_gfx_contexts();

int core_init() {
    const char *game_path = "./games/rhythm-land.gb";

    if (!cart_load(game_path)) {
        return -1;
    }

    cart_print_info();

    core_init_gfx_contexts();

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

void core_init_gfx_contexts() {
    const SDL_Color COLOR_BLACK = {0, 0, 0, 0};

    gfx_debug_ctx = gfx_init(256, 256, "Debug");
    gfx_clear(gfx_debug_ctx, COLOR_BLACK);
    gfx_update(gfx_debug_ctx);

    gfx_render_ctx = gfx_init(LCD_PIXELS_WIDTH * PPU_RENDER_SCALE, LCD_PIXELS_HEIGHT * PPU_RENDER_SCALE, "Goodboy");
    gfx_clear(gfx_render_ctx, COLOR_BLACK);
    gfx_update(gfx_render_ctx);
}
