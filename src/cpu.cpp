#include "cpu.h"
#include "cpu_instructions.h"
#include "cpu_routines.h"
#include "emulator_core.h"
#include "memory_bus.h"
#include <stdio.h>

gb_cpu_registers cpu_registers;
bool cpu_interrupt_master_enable = false;
uint8_t cpu_current_op_code = 0;
uint32_t cpu_instruction_counter = 0;
cpu_execute_op cpu_current_instruction_execute = nullptr;

void cpu_reset() {
    // After executing boot rom registers should have these values
    cpu_registers.af = 0x01B0;
    cpu_registers.bc = 0x0013;
    cpu_registers.de = 0x00D8;
    cpu_registers.hl = 0x014D;
    cpu_registers.sp = 0xFFFE;
    cpu_registers.pc = 0x0100;
}

void cpu_fetch() {
    // TODO: Read from memory bus instead of directly from ROM data
    cpu_current_op_code = memory_bus_read(cpu_registers.pc++);
    const gb_cpu_instruction &instruction = instructions[cpu_current_op_code];
    cpu_current_instruction_execute = instruction.execute;
}

bool cpu_execute() {
    if (!cpu_current_instruction_execute) {
        const gb_cpu_instruction &instruction = instructions[cpu_current_op_code];
        const uint8_t pchi = ((cpu_registers.pc - 1) & 0xFF00) >> 8;
        const uint8_t pclo = ((cpu_registers.pc - 1) & 0xFF);
        printf("Unknown instruction %.2X at: %.2X%.2X (%s), count %i\n", cpu_current_op_code, pchi, pclo,
               instruction.disassembly, cpu_instruction_counter);
        return false;
    }

    cpu_current_instruction_execute();

    return true;
}

// 0x00
void cpu_noop() { core_advance_cpu_clocks(4); }

// 0x01
void cpu_ld_bc_nn() { cpu_routine_ld_16(cpu_registers.b, cpu_registers.c); }

// 0x02
void cpu_ld_bc_a() {
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.bc, cpu_registers.a);
    core_advance_cpu_clocks(4);
}

// 0x03
void cpu_inc_bc() { cpu_routine_inc_16(cpu_registers.bc); }

// 0x04
void cpu_inc_b() { cpu_routine_inc_8(cpu_registers.b); }

// 0x05
void cpu_dec_b() { cpu_routine_dec_8(cpu_registers.b); }

// 0x06
void cpu_ld_b_n() { cpu_routine_ld_8(cpu_registers.b); }

// 0x0E
void cpu_ld_c_n() { cpu_routine_ld_8(cpu_registers.c); }

// 0x11
void cpu_ld_de_nn() { cpu_routine_ld_16(cpu_registers.d, cpu_registers.e); }

// 0x16
void cpu_ld_d_n() { cpu_routine_ld_8(cpu_registers.d); }

// 0x1E
void cpu_ld_e_n() { cpu_routine_ld_8(cpu_registers.e); }

// 0x21
void cpu_ld_hl_nn() { cpu_routine_ld_16(cpu_registers.h, cpu_registers.l); }

// 0x26
void cpu_ld_h_n() { cpu_routine_ld_8(cpu_registers.h); }

// 0x2E
void cpu_ld_l_n() { cpu_routine_ld_8(cpu_registers.l); }

// 0x31
void cpu_ld_sp_nn() { cpu_routine_ld_16(cpu_registers.s, cpu_registers.p); }

// 0x32
void cpu_ldd_hl_a() {
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.hl, cpu_registers.a);
    cpu_registers.hl = (cpu_registers.hl - 1) & 0xFFFF;
    core_advance_cpu_clocks(4);
}

// 0x3E
void cpu_ld_a_n() { cpu_routine_ld_8(cpu_registers.a); }

// 0xC3
void cpu_jp_nn() {
    core_advance_cpu_clocks(4);
    uint32_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8;
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    cpu_registers.pc = temp;
    core_advance_cpu_clocks(4);
}

// 0xF3
void cpu_di() {
    cpu_interrupt_master_enable = false;
    core_advance_cpu_clocks(4);
}
