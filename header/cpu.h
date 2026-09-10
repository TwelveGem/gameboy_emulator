#pragma once
#include <stdint.h>

struct gb_cpu_registers {
    struct {
        union {
            uint8_t f;
            uint8_t a;
        };
        uint16_t af;
    };

    struct {
        union {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };

    struct {
        union {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };

    struct {
        union {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };

    struct {
        union {
            uint8_t p;
            uint8_t s;
        };
        uint16_t sp; // Stack Pointer
    };
    uint16_t pc; // Program Counter
};

typedef void (*cpu_execute_op)();

extern gb_cpu_registers cpu_registers;

void cpu_reset();
void cpu_fetch();
bool cpu_execute();

// CPU Operations
void cpu_noop();  // 0x00
void cpu_jp_nn(); // 0xC3
