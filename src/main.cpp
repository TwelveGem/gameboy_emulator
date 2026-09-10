#include <cstdio>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "cart.h"
#include "cpu.h"

int SDLCALL goodboy_runapp_callback(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    return SDL_RunApp(argc, argv, goodboy_runapp_callback, NULL);
}

int SDLCALL goodboy_runapp_callback(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return -1;
    }

    printf("Hello, World!\n");

    const char *game_path = "./games/rhythm-land.gb";

    if (cart_load(game_path)) {
        cart_print_info();
    }

    cpu_reset();
    cpu_fetch();
    cpu_execute();

    SDL_Quit();

    return 0;
}
