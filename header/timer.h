#pragma once

#include <stdint.h>

struct gb_timer_registers {
    uint8_t timer_div;
    uint8_t timer_tima;
    uint8_t timer_tma;
    uint8_t timer_tac;
};

extern gb_timer_registers *timer_registers;

void timer_init();
void timer_advance_clocks(const uint8_t cycles);
void timer_on_div_write(const uint8_t _value);
void timer_tick_tima();