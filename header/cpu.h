#pragma once
#include <stdint.h>

#define SET_FLAG_ZERO(value)       cpu_registers.f = (cpu_registers.f & ~(1UL << 7)) | ((value) << 7);
#define GET_FLAG_ZERO              ((cpu_registers.f & (1UL << 7)) >> 7)
#define SET_FLAG_SUBTRACT(value)   cpu_registers.f = (cpu_registers.f & ~(1UL << 6)) | ((value) << 6);
#define GET_FLAG_SUBTRACT          ((cpu_registers.f & (1UL << 6)) >> 6)
#define SET_FLAG_HALF_CARRY(value) cpu_registers.f = (cpu_registers.f & ~(1UL << 5)) | ((value) << 5);
#define GET_FLAG_HALF_CARRY        ((cpu_registers.f & (1UL << 5)) >> 5)
#define SET_FLAG_CARRY(value)      cpu_registers.f = (cpu_registers.f & ~(1UL << 4)) | ((value) << 4);
#define GET_FLAG_CARRY             ((cpu_registers.f & (1UL << 4)) >> 4)

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
// 0x0
void cpu_noop();      // 0x00
void cpu_ld_bc_nn();  // 0x01
void cpu_ld_bc_a();   // 0x02
void cpu_inc_bc();    // 0x03
void cpu_inc_b();     // 0x04
void cpu_dec_b();     // 0x05
void cpu_ld_b_n();    // 0x06
void cpu_add_hl_bc(); // 0x09
void cpu_ld_a_bc();   // 0x0A
void cpu_dec_bc();    // 0x0B
void cpu_inc_c();     // 0x0C
void cpu_dec_c();     // 0x0D
void cpu_ld_c_n();    // 0x0E

// 0x1
void cpu_ld_de_nn();  // 0x11
void cpu_ld_de_a();   // 0x12
void cpu_inc_de();    // 0x13
void cpu_inc_d();     // 0x14
void cpu_dec_d();     // 0x15
void cpu_ld_d_n();    // 0x16
void cpu_add_hl_de(); // 0x19
void cpu_ld_a_de();   // 0x1A
void cpu_dec_de();    // 0x1B
void cpu_inc_e();     // 0x1C
void cpu_dec_e();     // 0x1D
void cpu_ld_e_n();    // 0x1E

// 0x2
void cpu_ld_hl_nn();  // 0x21
void cpu_inc_hl();    // 0x23
void cpu_inc_h();     // 0x24
void cpu_dec_h();     // 0x25
void cpu_ld_h_n();    // 0x26
void cpu_add_hl_hl(); // 0x29
void cpu_dec_hl();    // 0x2B
void cpu_inc_l();     // 0x2C
void cpu_dec_l();     // 0x2D
void cpu_ld_l_n();    // 0x2E

// 0x3
void cpu_ld_sp_nn();  // 0x31
void cpu_ldd_hl_a();  // 0x32
void cpu_inc_sp();    // 0x33
void cpu_add_hl_sp(); // 0x39
void cpu_dec_sp();    // 0x3B
void cpu_inc_a();     // 0x3C
void cpu_dec_a();     // 0x3D
void cpu_ld_a_n();    // 0x3E

// 0x4
void cpu_ld_b_hl(); // 0x46
void cpu_ld_c_hl(); // 0x4E

// 0x5
void cpu_ld_d_hl(); // 0x56
void cpu_ld_e_hl(); // 0x5E

// 0x6
void cpu_ld_h_hl(); // 0x66
void cpu_ld_l_hl(); // 0x6E

// 0x7
void cpu_ld_hl_b(); // 0x70
void cpu_ld_hl_c(); // 0x71
void cpu_ld_hl_d(); // 0x72
void cpu_ld_hl_e(); // 0x73
void cpu_ld_hl_h(); // 0x74
void cpu_ld_hl_l(); // 0x75
void cpu_ld_hl_a(); // 0x77
void cpu_ld_a_hl(); // 0x7E

// 0x8
void cpu_add_a_b(); // 0x80
void cpu_add_a_c(); // 0x81
void cpu_add_a_d(); // 0x82
void cpu_add_a_e(); // 0x83
void cpu_add_a_h(); // 0x84
void cpu_add_a_l(); // 0x85
void cpu_add_a_a(); // 0x87
void cpu_adc_a_b(); // 0x88
void cpu_adc_a_c(); // 0x89
void cpu_adc_a_d(); // 0x8A
void cpu_adc_a_e(); // 0x8B
void cpu_adc_a_h(); // 0x8C
void cpu_adc_a_l(); // 0x8D

// 0x8

// 0x9
void cpu_sub_a_b(); // 0x90
void cpu_sub_a_c(); // 0x91
void cpu_sub_a_d(); // 0x92
void cpu_sub_a_e(); // 0x93
void cpu_sub_a_h(); // 0x94
void cpu_sub_a_l(); // 0x95
void cpu_sbc_a_b(); // 0x98
void cpu_sbc_a_c(); // 0x99
void cpu_sbc_a_d(); // 0x9A
void cpu_sbc_a_e(); // 0x9B
void cpu_sbc_a_h(); // 0x9C
void cpu_sbc_a_l(); // 0x9D

// 0xA
// 0xB

// 0xC
void cpu_jp_nn(); // 0xC3

// 0xD
// 0xE

// 0xF
void cpu_di(); // 0xF3
