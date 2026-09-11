#pragma once
#include <stdint.h>

#define SET_FLAG_ZERO(value)       cpu_registers.f = (cpu_registers.f & ~(1UL << 7)) | ((value) << 7);
#define GET_FLAG_ZERO(value)       ((cpu_registers.f & ~(1UL << 7)) >> 7)
#define SET_FLAG_SUBTRACT(value)   cpu_registers.f = (cpu_registers.f & ~(1UL << 6)) | ((value) << 6);
#define GET_FLAG_SUBTRACT(value)   ((cpu_registers.f & ~(1UL << 6)) >> 6)
#define SET_FLAG_HALF_CARRY(value) cpu_registers.f = (cpu_registers.f & ~(1UL << 5)) | ((value) << 5);
#define GET_FLAG_HALF_CARRY(value) ((cpu_registers.f & ~(1UL << 5)) >> 5)
#define SET_FLAG_CARRY(value)      cpu_registers.f = (cpu_registers.f & ~(1UL << 4)) | ((value) << 4);
#define GET_FLAG_CARRY(value)      ((cpu_registers.f & ~(1UL << 4)) >> 4)

struct gb_cpu_registers {
    union {
        struct {
            uint8_t f;
            uint8_t a;
        };
        uint16_t af;
    };

    union {
        struct {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };

    union {
        struct {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };

    union {
        struct {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };

    union {
        struct {
            uint8_t p;
            uint8_t s;
        };
        uint16_t sp; // Stack Pointer
    };
    uint16_t pc; // Program Counter
};

typedef void (*cpu_execute_op)();

extern gb_cpu_registers cpu_registers;
extern bool cpu_interrupt_master_enable;

void cpu_reset();
void cpu_fetch();
bool cpu_execute();

// CPU Operations
void cpu_noop();     // 0x00
void cpu_ld_bc_nn(); // 0x01
void cpu_ld_bc_a();  // 0x02
void cpu_inc_bc();   // 0x03
void cpu_inc_b();    // 0x04
void cpu_dec_b();    // 0x05
void cpu_ld_b_n();   // 0x06
void cpu_ld_hl_nn(); // 0x21
void cpu_ldd_hl_a(); // 0x32
void cpu_jp_nn();    // 0xC3
void cpu_ld_a_n();   // 0xF0
void cpu_di();       // 0xF3
