#include "cpu.h"
#include "cpu_instructions.h"
#include "cpu_routines.h"
#include "emulator_core.h"
#include "interrupts.h"
#include "memory_bus.h"
#include "timer.h"
#include <stdio.h>

gb_cpu_registers cpu_registers;
uint8_t cpu_current_op_code = 0;
uint32_t cpu_instruction_counter = 0;
cpu_execute_op cpu_current_instruction_execute = nullptr;
uint8_t cpu_halt_count = 0; // 0 == not halted, 1 == halt instruction, 2 == stop instruction
bool cpu_halt_bug = false;

void cpu_reset() {
    // After executing boot rom registers should have these values
    cpu_registers.af = 0x01B0;
    cpu_registers.bc = 0x0013;
    cpu_registers.de = 0x00D8;
    cpu_registers.hl = 0x014D;
    cpu_registers.sp = 0xFFFE;
    cpu_registers.pc = 0x0100;
}

void cpu_tick() {
    if (cpu_halt_count == 0) {
        cpu_fetch();
        cpu_execute();
        cpu_instruction_counter++;
    } else {
        core_advance_cpu_clocks(4);
    }

    interrupt_service_routine();
}

void cpu_fetch() {
    cpu_current_op_code = memory_bus_read(cpu_registers.pc++);
    const gb_cpu_instruction &instruction = instructions[cpu_current_op_code];
    cpu_current_instruction_execute = instruction.execute;

    if (cpu_halt_bug) {
        cpu_registers.pc--; // Repeat one byte during halt bug
        cpu_halt_bug = false;
    }
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
void cpu_ld_bc_a() { cpu_routine_ld_ptr8(cpu_registers.bc, cpu_registers.a); }

// 0x03
void cpu_inc_bc() { cpu_routine_inc_16(cpu_registers.bc); }

// 0x04
void cpu_inc_b() { cpu_routine_inc_8(cpu_registers.b); }

// 0x05
void cpu_dec_b() { cpu_routine_dec_8(cpu_registers.b); }

// 0x06
void cpu_ld_b_n() { cpu_routine_ld_8(cpu_registers.b); }

// 0x07
void cpu_rlca() {
    core_advance_cpu_clocks(4);
    SET_FLAG_ZERO(0);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(0);
    uint8_t carry = (cpu_registers.a & 0x80) > 0;
    cpu_registers.a = cpu_registers.a << 1 | carry;
    SET_FLAG_CARRY(carry);
}

// 0x08
void cpu_ld_nn_sp() {
    core_advance_cpu_clocks(4);
    uint16_t addr = memory_bus_read(cpu_registers.pc++);
    core_advance_cpu_clocks(4);
    addr |= ((uint16_t)memory_bus_read(cpu_registers.pc++)) << 8;
    core_advance_cpu_clocks(4);
    memory_bus_write(addr++, (cpu_registers.sp & 0xFF));
    core_advance_cpu_clocks(4);
    memory_bus_write(addr, ((cpu_registers.sp & 0xFF00) >> 8));
    core_advance_cpu_clocks(4);
}

// 0x09
void cpu_add_hl_bc() { cpu_routine_add_hl_16(cpu_registers.bc); }

// 0x0A
void cpu_ld_a_bc() { cpu_routine_ld_ptr_16(cpu_registers.a, cpu_registers.bc); }

// 0x0B
void cpu_dec_bc() { cpu_routine_dec_16(cpu_registers.bc); }

// 0x0C
void cpu_inc_c() { cpu_routine_inc_8(cpu_registers.c); }

// 0x0D
void cpu_dec_c() { cpu_routine_dec_8(cpu_registers.c); }

// 0x0E
void cpu_ld_c_n() { cpu_routine_ld_8(cpu_registers.c); }

// 0x0F
void cpu_rrca() {
    core_advance_cpu_clocks(4);
    SET_FLAG_ZERO(0);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(0);
    uint8_t carry = cpu_registers.a & 0x01;
    cpu_registers.a = cpu_registers.a >> 1 | carry * 0x80;
    SET_FLAG_CARRY(carry);
}

// 0x10
void cpu_stop() {
    core_advance_cpu_clocks(4);
    if (memory_bus_read(cpu_registers.pc++) != 0) {
        printf("CPU - Corrupted STOP at PC: %04X, should have operand 0x00\n", cpu_registers.pc);
    }

    core_advance_cpu_clocks(4);
    timer_on_div_write(0);
    cpu_halt_count = 2;
}

// 0x11
void cpu_ld_de_nn() { cpu_routine_ld_16(cpu_registers.d, cpu_registers.e); }

// 0x12
void cpu_ld_de_a() { cpu_routine_ld_ptr8(cpu_registers.de, cpu_registers.a); }

// 0x13
void cpu_inc_de() { cpu_routine_inc_16(cpu_registers.de); }

// 0x14
void cpu_inc_d() { cpu_routine_inc_8(cpu_registers.d); }

// 0x15
void cpu_dec_d() { cpu_routine_dec_8(cpu_registers.d); }

// 0x16
void cpu_ld_d_n() { cpu_routine_ld_8(cpu_registers.d); }

// 0x17
void cpu_rla() {
    core_advance_cpu_clocks(4);
    SET_FLAG_ZERO(0);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(0);
    SET_FLAG_CARRY((cpu_registers.a & 0x80) > 0);
    cpu_registers.a = (cpu_registers.a << 1) | GET_FLAG_CARRY;
}

// 0x18
void cpu_jr_n() { cpu_routine_jr_conditional_n(1); }

// 0x19
void cpu_add_hl_de() { cpu_routine_add_hl_16(cpu_registers.de); }

// 0x1A
void cpu_ld_a_de() { cpu_routine_ld_ptr_16(cpu_registers.a, cpu_registers.de); }

// 0x1B
void cpu_dec_de() { cpu_routine_dec_16(cpu_registers.de); }

// 0x1C
void cpu_inc_e() { cpu_routine_inc_8(cpu_registers.e); }

// 0x1D
void cpu_dec_e() { cpu_routine_dec_8(cpu_registers.e); }

// 0x1E
void cpu_ld_e_n() { cpu_routine_ld_8(cpu_registers.e); }

// 0x1F
void cpu_rra() {
    core_advance_cpu_clocks(4);
    SET_FLAG_ZERO(0);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(0);
    SET_FLAG_CARRY(cpu_registers.a & 0x01);
    cpu_registers.a = cpu_registers.a >> 1 | GET_FLAG_CARRY * 0x80;
}

// 0x20
void cpu_jr_nz() { cpu_routine_jr_conditional_n(GET_FLAG_ZERO == 0x00); }

// 0x21
void cpu_ld_hl_nn() { cpu_routine_ld_16(cpu_registers.h, cpu_registers.l); }

// 0x22
void cpu_ldi_hl_a() {
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.hl, cpu_registers.a);
    core_advance_cpu_clocks(4);
    cpu_registers.hl = (cpu_registers.hl + 1) & 0xFFFF;
}

// 0x23
void cpu_inc_hl() { cpu_routine_inc_16(cpu_registers.hl); }

// 0x24
void cpu_inc_h() { cpu_routine_inc_8(cpu_registers.h); }

// 0x25
void cpu_dec_h() { cpu_routine_dec_8(cpu_registers.h); }

// 0x26
void cpu_ld_h_n() { cpu_routine_ld_8(cpu_registers.h); }

// 0x27
void cpu_daa() {
    core_advance_cpu_clocks(4);

    if (!GET_FLAG_SUBTRACT) {
        // after an addition, adjust if (half-) carry orrurd or if result is out of bounds
        if (GET_FLAG_CARRY || cpu_registers.a > 0x99) {
            cpu_registers.a += 0x60;
            SET_FLAG_CARRY(1);
        }

        if (GET_FLAG_HALF_CARRY || (cpu_registers.a & 0x0F) > 0x09) {
            cpu_registers.a += 0x6;
        }
    } else {
        // after a subtraction, only adjust if (half-) carry occurred
        if (GET_FLAG_CARRY) {
            cpu_registers.a -= 0x60;
        }
        if (GET_FLAG_HALF_CARRY) {
            cpu_registers.a -= 0x6;
        }
    }
    SET_FLAG_ZERO(cpu_registers.a == 0);
    SET_FLAG_HALF_CARRY(0);
}

// 0x28
void cpu_jr_z() { cpu_routine_jr_conditional_n(GET_FLAG_ZERO); }

// 0x29
void cpu_add_hl_hl() {
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_CARRY((cpu_registers.hl & 0x8000) != 0);
    SET_FLAG_HALF_CARRY((cpu_registers.hl & 0x0800) != 0);
    core_advance_cpu_clocks(4);
    cpu_registers.hl = (cpu_registers.hl << 1) & 0xFFFF;
}

// 0x2A
void cpu_ldi_a_hl() {
    core_advance_cpu_clocks(4);
    cpu_registers.a = memory_bus_read(cpu_registers.hl);
    core_advance_cpu_clocks(4);
    cpu_registers.hl = (cpu_registers.hl + 1) & 0xFFFF;
}

// 0x2B
void cpu_dec_hl() { cpu_routine_dec_16(cpu_registers.hl); }

// 0x2C
void cpu_inc_l() { cpu_routine_inc_8(cpu_registers.l); }

// 0x2D
void cpu_dec_l() { cpu_routine_dec_8(cpu_registers.l); }

// 0x2E
void cpu_ld_l_n() { cpu_routine_ld_8(cpu_registers.l); }

// 0x2F
void cpu_cpl() {
    core_advance_cpu_clocks(4);
    cpu_registers.a = ~cpu_registers.a;
    SET_FLAG_SUBTRACT(1);
    SET_FLAG_HALF_CARRY(1);
}

// 0x30
void cpu_jr_nc() { cpu_routine_jr_conditional_n(GET_FLAG_CARRY == 0x00); }

// 0x31
void cpu_ld_sp_nn() { cpu_routine_ld_16(cpu_registers.s, cpu_registers.p); }

// 0x32
void cpu_ldd_hl_a() {
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.hl, cpu_registers.a);
    cpu_registers.hl = (cpu_registers.hl - 1) & 0xFFFF;
    core_advance_cpu_clocks(4);
}

// 0x33
void cpu_inc_sp() { cpu_routine_inc_16(cpu_registers.sp); }

// 0x34
void cpu_inc__hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(0);
    temp = (temp + 1) & 0xFF;
    SET_FLAG_HALF_CARRY((temp & 0xF) == 0);
    SET_FLAG_ZERO(temp == 0);
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.hl, temp);
}

// 0x35
void cpu_dec__hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(1);
    SET_FLAG_HALF_CARRY((temp & 0xF) == 0);
    temp = (temp - 1) & 0xFF;
    SET_FLAG_ZERO(temp == 0);
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.hl, temp);
}

// 0x36
void cpu_ld_hl_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.hl, temp);
    core_advance_cpu_clocks(4);
}

// 0x37
void cpu_scf() {
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(0);
    SET_FLAG_CARRY(1);
    core_advance_cpu_clocks(4);
}

// 0x38
void cpu_jr_c() { cpu_routine_jr_conditional_n(GET_FLAG_CARRY); }

// 0x39
void cpu_add_hl_sp() { cpu_routine_add_hl_16(cpu_registers.sp); }

// 0x3A
void cpu_ldd_a_hl() {
    core_advance_cpu_clocks(4);
    cpu_registers.a = memory_bus_read(cpu_registers.hl);
    core_advance_cpu_clocks(4);
    cpu_registers.hl = (cpu_registers.hl - 1) & 0xFFFF;
}

// 0x3B
void cpu_dec_sp() { cpu_routine_dec_16(cpu_registers.sp); }

// 0x3C
void cpu_inc_a() { cpu_routine_inc_8(cpu_registers.a); }

// 0x3D
void cpu_dec_a() { cpu_routine_dec_8(cpu_registers.a); }

// 0x3E
void cpu_ld_a_n() { cpu_routine_ld_8(cpu_registers.a); }

// 0x3F
void cpu_ccf() {
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(0);
    SET_FLAG_CARRY(!GET_FLAG_CARRY);
    core_advance_cpu_clocks(4);
}

// 0x40
void cpu_ld_b_b() {
    cpu_registers.b = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x41
void cpu_ld_b_c() {
    cpu_registers.b = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x42
void cpu_ld_b_d() {
    cpu_registers.b = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x43
void cpu_ld_b_e() {
    cpu_registers.b = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x44
void cpu_ld_b_h() {
    cpu_registers.b = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x45
void cpu_ld_b_l() {
    cpu_registers.b = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x46
void cpu_ld_b_hl() { cpu_routine_ld_ptr_16(cpu_registers.b, cpu_registers.hl); }

// 0x47
void cpu_ld_b_a() {
    cpu_registers.b = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x48
void cpu_ld_c_b() {
    cpu_registers.c = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x49
void cpu_ld_c_c() {
    cpu_registers.c = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x4A
void cpu_ld_c_d() {
    cpu_registers.c = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x4B
void cpu_ld_c_e() {
    cpu_registers.c = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x4C
void cpu_ld_c_h() {
    cpu_registers.c = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x4D
void cpu_ld_c_l() {
    cpu_registers.c = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x4E
void cpu_ld_c_hl() { cpu_routine_ld_ptr_16(cpu_registers.c, cpu_registers.hl); }

// 0x4F
void cpu_ld_c_a() {
    cpu_registers.c = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x50
void cpu_ld_d_b() {
    cpu_registers.d = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x51
void cpu_ld_d_c() {
    cpu_registers.d = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x52
void cpu_ld_d_d() {
    cpu_registers.d = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x53
void cpu_ld_d_e() {
    cpu_registers.d = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x54
void cpu_ld_d_h() {
    cpu_registers.d = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x55
void cpu_ld_d_l() {
    cpu_registers.d = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x56
void cpu_ld_d_hl() { cpu_routine_ld_ptr_16(cpu_registers.d, cpu_registers.hl); }

// 0x57
void cpu_ld_d_a() {
    cpu_registers.d = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x58
void cpu_ld_e_b() {
    cpu_registers.e = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x59
void cpu_ld_e_c() {
    cpu_registers.e = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x5A
void cpu_ld_e_d() {
    cpu_registers.e = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x5B
void cpu_ld_e_e() {
    cpu_registers.e = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x5C
void cpu_ld_e_h() {
    cpu_registers.e = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x5D
void cpu_ld_e_l() {
    cpu_registers.e = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x5E
void cpu_ld_e_hl() { cpu_routine_ld_ptr_16(cpu_registers.e, cpu_registers.hl); }

// 0x5F
void cpu_ld_e_a() {
    cpu_registers.e = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x60
void cpu_ld_h_b() {
    cpu_registers.h = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x61
void cpu_ld_h_c() {
    cpu_registers.h = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x62
void cpu_ld_h_d() {
    cpu_registers.h = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x63
void cpu_ld_h_e() {
    cpu_registers.h = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x64
void cpu_ld_h_h() {
    cpu_registers.h = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x65
void cpu_ld_h_l() {
    cpu_registers.h = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x66
void cpu_ld_h_hl() { cpu_routine_ld_ptr_16(cpu_registers.h, cpu_registers.hl); }

// 0x67
void cpu_ld_h_a() {
    cpu_registers.h = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x68
void cpu_ld_l_b() {
    cpu_registers.l = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x69
void cpu_ld_l_c() {
    cpu_registers.l = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x6A
void cpu_ld_l_d() {
    cpu_registers.l = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x6B
void cpu_ld_l_e() {
    cpu_registers.l = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x6C
void cpu_ld_l_h() {
    cpu_registers.l = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x6D
void cpu_ld_l_l() {
    cpu_registers.l = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x6E
void cpu_ld_l_hl() { cpu_routine_ld_ptr_16(cpu_registers.l, cpu_registers.hl); }

// 0x6F
void cpu_ld_l_a() {
    cpu_registers.l = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x70
void cpu_ld_hl_b() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.b); }

// 0x71
void cpu_ld_hl_c() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.c); }

// 0x72
void cpu_ld_hl_d() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.d); }

// 0x73
void cpu_ld_hl_e() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.e); }

// 0x74
void cpu_ld_hl_h() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.h); }

// 0x75
void cpu_ld_hl_l() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.l); }

void cpu_halt() {
    core_advance_cpu_clocks(4);
    const uint8_t interrupt_enable = memory_bus_read(ADDR_IO_IE);
    const uint8_t interrupt_flag = memory_bus_read(ADDR_IO_IF);
    const bool interrupt_pending = ((interrupt_enable & interrupt_flag) & 0x1F) != 0;
    if (!interrupt_master_enable && interrupt_pending != 0) {
        cpu_halt_bug = true;
    } else {
        cpu_halt_count = 1;
    }
}

// 0x77
void cpu_ld_hl_a() { cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.a); }

// 0x78
void cpu_ld_a_b() {
    cpu_registers.a = cpu_registers.b;
    core_advance_cpu_clocks(4);
}

// 0x79
void cpu_ld_a_c() {
    cpu_registers.a = cpu_registers.c;
    core_advance_cpu_clocks(4);
}

// 0x7A
void cpu_ld_a_d() {
    cpu_registers.a = cpu_registers.d;
    core_advance_cpu_clocks(4);
}

// 0x7B
void cpu_ld_a_e() {
    cpu_registers.a = cpu_registers.e;
    core_advance_cpu_clocks(4);
}

// 0x7C
void cpu_ld_a_h() {
    cpu_registers.a = cpu_registers.h;
    core_advance_cpu_clocks(4);
}

// 0x7D
void cpu_ld_a_l() {
    cpu_registers.a = cpu_registers.l;
    core_advance_cpu_clocks(4);
}

// 0x7E
void cpu_ld_a_hl() { cpu_routine_ld_ptr_16(cpu_registers.a, cpu_registers.hl); }

// 0x7F
void cpu_ld_a_a() {
    cpu_registers.a = cpu_registers.a;
    core_advance_cpu_clocks(4);
}

// 0x80
void cpu_add_a_b() { cpu_routine_add_a_8(cpu_registers.b); }

// 0x81
void cpu_add_a_c() { cpu_routine_add_a_8(cpu_registers.c); }

// 0x82
void cpu_add_a_d() { cpu_routine_add_a_8(cpu_registers.d); }

// 0x83
void cpu_add_a_e() { cpu_routine_add_a_8(cpu_registers.e); }

// 0x84
void cpu_add_a_h() { cpu_routine_add_a_8(cpu_registers.h); }

// 0x85
void cpu_add_a_l() { cpu_routine_add_a_8(cpu_registers.l); }

// 0x86
void cpu_add_a_hl() {
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(0);
    uint32_t temp_a = cpu_registers.a;
    uint32_t temp_hl = memory_bus_read(cpu_registers.hl);
    SET_FLAG_HALF_CARRY(((temp_a & 0xF) + (temp_hl & 0xF)) > 0xF);
    core_advance_cpu_clocks(4);
    cpu_registers.a += temp_hl;
    SET_FLAG_ZERO(cpu_registers.a == 0);
    SET_FLAG_CARRY(temp_a > cpu_registers.a);
}

// 0x87
void cpu_add_a_a() { cpu_routine_add_a_8(cpu_registers.a); }

// 0x88
void cpu_adc_a_b() { cpu_routine_adc_a_8(cpu_registers.b); }

// 0x89
void cpu_adc_a_c() { cpu_routine_adc_a_8(cpu_registers.c); }

// 0x8A
void cpu_adc_a_d() { cpu_routine_adc_a_8(cpu_registers.d); }

// 0x8B
void cpu_adc_a_e() { cpu_routine_adc_a_8(cpu_registers.e); }

// 0x8C
void cpu_adc_a_h() { cpu_routine_adc_a_8(cpu_registers.h); }

// 0x8D
void cpu_adc_a_l() { cpu_routine_adc_a_8(cpu_registers.l); }

// 0x8E
void cpu_adc_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_adc_a_8(temp);
}

// 0x8F
void cpu_adc_a_a() { cpu_routine_adc_a_8(cpu_registers.a); }

// 0x90
void cpu_sub_a_b() { cpu_routine_sub_a_8(cpu_registers.b); }

// 0x91
void cpu_sub_a_c() { cpu_routine_sub_a_8(cpu_registers.c); }

// 0x92
void cpu_sub_a_d() { cpu_routine_sub_a_8(cpu_registers.d); }

// 0x93
void cpu_sub_a_e() { cpu_routine_sub_a_8(cpu_registers.e); }

// 0x94
void cpu_sub_a_h() { cpu_routine_sub_a_8(cpu_registers.h); }

// 0x95
void cpu_sub_a_l() { cpu_routine_sub_a_8(cpu_registers.l); }

// 0x96
void cpu_sub_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_sub_a_8(temp);
}

// 0x97
void cpu_sub_a_a() { cpu_routine_sub_a_8(cpu_registers.a); }

// 0x98
void cpu_sbc_a_b() { cpu_routine_sbc_a_8(cpu_registers.b); }

// 0x99
void cpu_sbc_a_c() { cpu_routine_sbc_a_8(cpu_registers.c); }

// 0x9A
void cpu_sbc_a_d() { cpu_routine_sbc_a_8(cpu_registers.d); }

// 0x9B
void cpu_sbc_a_e() { cpu_routine_sbc_a_8(cpu_registers.e); }

// 0x9C
void cpu_sbc_a_h() { cpu_routine_sbc_a_8(cpu_registers.h); }

// 0x9D
void cpu_sbc_a_l() { cpu_routine_sbc_a_8(cpu_registers.l); }

// 0x9E
void cpu_sbc_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_sbc_a_8(temp);
}

// 0x9F
void cpu_sbc_a_a() { cpu_routine_sbc_a_8(cpu_registers.a); }

// 0xA0
void cpu_and_a_b() { cpu_routine_and_a_8(cpu_registers.b); }

// 0xA1
void cpu_and_a_c() { cpu_routine_and_a_8(cpu_registers.c); }

// 0xA2
void cpu_and_a_d() { cpu_routine_and_a_8(cpu_registers.d); }

// 0xA3
void cpu_and_a_e() { cpu_routine_and_a_8(cpu_registers.e); }

// 0xA4
void cpu_and_a_h() { cpu_routine_and_a_8(cpu_registers.h); }

// 0xA5
void cpu_and_a_l() { cpu_routine_and_a_8(cpu_registers.l); }

// 0xA6
void cpu_and_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_and_a_8(temp);
}

// 0xA7
void cpu_and_a_a() { cpu_routine_and_a_8(cpu_registers.a); }

// 0xA8
void cpu_xor_a_b() { cpu_routine_xor_a_8(cpu_registers.b); }

// 0xA9
void cpu_xor_a_c() { cpu_routine_xor_a_8(cpu_registers.c); }

// 0xAA
void cpu_xor_a_d() { cpu_routine_xor_a_8(cpu_registers.d); }

// 0xAB
void cpu_xor_a_e() { cpu_routine_xor_a_8(cpu_registers.e); }

// 0xAC
void cpu_xor_a_h() { cpu_routine_xor_a_8(cpu_registers.h); }

// 0xAD
void cpu_xor_a_l() { cpu_routine_xor_a_8(cpu_registers.l); }

// 0xAE
void cpu_xor_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_xor_a_8(temp);
}

// 0xAF
void cpu_xor_a_a() { cpu_routine_xor_a_8(cpu_registers.a); }

// 0xB0
void cpu_or_a_b() { cpu_routine_or_a_8(cpu_registers.b); }

// 0xB1
void cpu_or_a_c() { cpu_routine_or_a_8(cpu_registers.c); }

// 0xB2
void cpu_or_a_d() { cpu_routine_or_a_8(cpu_registers.d); }

// 0xB3
void cpu_or_a_e() { cpu_routine_or_a_8(cpu_registers.e); }

// 0xB4
void cpu_or_a_h() { cpu_routine_or_a_8(cpu_registers.h); }

// 0xB5
void cpu_or_a_l() { cpu_routine_or_a_8(cpu_registers.l); }

// 0xB6
void cpu_or_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_or_a_8(temp);
}

// 0xB7
void cpu_or_a_a() { cpu_routine_or_a_8(cpu_registers.a); }

// 0xB8
void cpu_cp_a_b() { cpu_routine_cp_a_8(cpu_registers.b); }

// 0xB9
void cpu_cp_a_c() { cpu_routine_cp_a_8(cpu_registers.c); }

// 0xBA
void cpu_cp_a_d() { cpu_routine_cp_a_8(cpu_registers.d); }

// 0xBB
void cpu_cp_a_e() { cpu_routine_cp_a_8(cpu_registers.e); }

// 0xBC
void cpu_cp_a_h() { cpu_routine_cp_a_8(cpu_registers.h); }

// 0xBD
void cpu_cp_a_l() { cpu_routine_cp_a_8(cpu_registers.l); }

// 0xBE
void cpu_cp_a_hl() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.hl);
    cpu_routine_cp_a_8(temp);
}

// 0xBF
void cpu_cp_a_a() { cpu_routine_cp_a_8(cpu_registers.a); }

// 0xC0
void cpu_ret_nz() { cpu_routine_return_conditional(GET_FLAG_ZERO == 0x00); }

// 0xC1
void cpu_pop_bc() {
    cpu_routine_pop_16(cpu_registers.b, cpu_registers.c);
    cpu_registers.f &= 0xF0; // All flags are reset
}

// 0xC2
void cpu_jp_nz() { cpu_routine_jump_conditional_nnnn(GET_FLAG_ZERO == 0x00); }

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

// 0xC4
void cpu_call_nz_nn() { cpu_routine_call_conditional_nnnn(GET_FLAG_ZERO == 0x00); }

// 0xC5
void cpu_push_bc() { cpu_routine_push_16(cpu_registers.b, cpu_registers.c); }

// 0xC6
void cpu_add_a_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    cpu_routine_add_a_8(temp);
}

// 0xC7
void cpu_rst_00() { cpu_routine_rst_nnnn(0x0000); }

// 0xC8
void cpu_ret_z() { cpu_routine_return_conditional(GET_FLAG_ZERO); }

// 0xC9
void cpu_ret() { cpu_routine_return_conditional(1); }

// 0xCA
void cpu_jp_z() { cpu_routine_jump_conditional_nnnn(GET_FLAG_ZERO); }

// 0xCC
void cpu_call_z_nn() { cpu_routine_call_conditional_nnnn(GET_FLAG_ZERO); }

// 0xCD
void cpu_call_nn() { cpu_routine_call_conditional_nnnn(1); }

// 0xCE
void cpu_adc_a_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    cpu_routine_adc_a_8(temp);
}

// 0xCF
void cpu_rst_08() { cpu_routine_rst_nnnn(0x0008); }

// 0xD0
void cpu_ret_nc() { cpu_routine_return_conditional(GET_FLAG_CARRY == 0x00); }

// 0xD1
void cpu_pop_de() {
    cpu_routine_pop_16(cpu_registers.d, cpu_registers.e);
    cpu_registers.f &= 0xF0; // All flags are reset
}

// 0xD2
void cpu_jp_nc() { cpu_routine_jump_conditional_nnnn(GET_FLAG_CARRY == 0x00); }

// 0xD4
void cpu_call_nc_nn() { cpu_routine_call_conditional_nnnn(GET_FLAG_CARRY == 0x00); }

// 0xD5
void cpu_push_de() { cpu_routine_push_16(cpu_registers.d, cpu_registers.e); }

// 0xD6
void cpu_sub_a_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    cpu_routine_sub_a_8(temp);
}

// 0xD7
void cpu_rst_10() { cpu_routine_rst_nnnn(0x0010); }

// 0xD8
void cpu_ret_c() { cpu_routine_return_conditional(GET_FLAG_CARRY); }

// 0xD9
void cpu_reti() {
    cpu_routine_return_conditional(true);
    interrupt_master_enable = true;
}

// 0xDA
void cpu_jp_c() { cpu_routine_jump_conditional_nnnn(GET_FLAG_CARRY); }

// 0xDC
void cpu_call_c_nn() { cpu_routine_call_conditional_nnnn(GET_FLAG_CARRY); }

// 0xDE
void cpu_sbc_a_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    cpu_routine_sbc_a_8(temp);
}

// 0xDF
void cpu_rst_18() { cpu_routine_rst_nnnn(0x0018); }

// 0xE0
void cpu_ldh_n_a() {
    core_advance_cpu_clocks(4);
    uint8_t addr = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    memory_bus_write(0xFF00 | addr, cpu_registers.a);
    core_advance_cpu_clocks(4);
}

// 0xE1
void cpu_pop_hl() {
    cpu_routine_pop_16(cpu_registers.h, cpu_registers.l);
    cpu_registers.f &= 0xF0; // All flags are reset
}

// 0xE2
void cpu_ldh_c_a() {
    core_advance_cpu_clocks(4);
    memory_bus_write(0xFF00 | cpu_registers.c, cpu_registers.a);
    core_advance_cpu_clocks(4);
}

// 0xE5
void cpu_push_hl() { cpu_routine_push_16(cpu_registers.h, cpu_registers.l); }

// 0xE6
void cpu_and_n() {
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_CARRY(0);
    SET_FLAG_HALF_CARRY(1);
    core_advance_cpu_clocks(4);
    cpu_registers.a &= memory_bus_read(cpu_registers.pc++);
    SET_FLAG_ZERO(cpu_registers.a == 0);
}

// 0xE7
void cpu_rst_20() { cpu_routine_rst_nnnn(0x0020); }

// 0xE8
void cpu_add_sp_d() {
    core_advance_cpu_clocks(4);
    int8_t temp = memory_bus_read(cpu_registers.pc++);
    core_advance_cpu_clocks(4);
    SET_FLAG_ZERO(0);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_CARRY(((cpu_registers.sp & 0x00FF) + (temp & 0x00FF)) > 0x00FF);
    SET_FLAG_HALF_CARRY(((cpu_registers.sp & 0x000F) + (temp & 0x000F)) > 0x000F);
    core_advance_cpu_clocks(4);
    cpu_registers.sp += temp;
    core_advance_cpu_clocks(4);
}

// 0xE9
void cpu_jp_hl() {
    core_advance_cpu_clocks(4);
    cpu_registers.pc = cpu_registers.hl;
}

// 0xEA
void cpu_ld_nn_a() {
    core_advance_cpu_clocks(4);
    uint32_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8;
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    memory_bus_write(temp, cpu_registers.a);
    core_advance_cpu_clocks(4);
}

// 0xEE
void cpu_xor_a_n() {
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_CARRY(0);
    SET_FLAG_HALF_CARRY(0);
    core_advance_cpu_clocks(4);
    cpu_registers.a ^= memory_bus_read(cpu_registers.pc++);
    SET_FLAG_ZERO(cpu_registers.a == 0);
}

// 0xEF
void cpu_rst_28() { cpu_routine_rst_nnnn(0x0028); }

// 0xF0
void cpu_ldh_a_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    core_advance_cpu_clocks(4);
    cpu_registers.a = memory_bus_read(0xFF00 | temp);
    core_advance_cpu_clocks(4);
}

// 0xF1
void cpu_pop_af() {
    cpu_routine_pop_16(cpu_registers.a, cpu_registers.f);
    cpu_registers.f &= 0xF0; // All flags are reset
}

// 0xF2
void cpu_ldh_a_c() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.c);
    core_advance_cpu_clocks(4);
    cpu_registers.a = memory_bus_read(temp);
    core_advance_cpu_clocks(4);
}

// 0xF3
void cpu_di() {
    core_advance_cpu_clocks(4);
    interrupt_master_enable = false;
    interrupt_enable_ime_delay = 0;
}

// 0xF5
void cpu_push_af() { cpu_routine_push_16(cpu_registers.a, cpu_registers.f); }

// 0xF6
void cpu_or_a_n() {
    core_advance_cpu_clocks(4);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_CARRY(0);
    SET_FLAG_HALF_CARRY(0);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    core_advance_cpu_clocks(4);
    cpu_registers.a |= temp;
    SET_FLAG_ZERO(cpu_registers.a == 0);
    core_advance_cpu_clocks(4);
}

// 0xF7
void cpu_rst_30() { cpu_routine_rst_nnnn(0x0038); }

// 0xF8
void cpu_ld_hl_sp_d() {
    core_advance_cpu_clocks(4);
    int8_t temp = (int8_t)memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    int16_t res = (int16_t)cpu_registers.sp + temp;
    core_advance_cpu_clocks(4);
    cpu_registers.hl = res & 0xFFFF;
    SET_FLAG_ZERO(0);
    SET_FLAG_SUBTRACT(0);
    SET_FLAG_HALF_CARRY(((cpu_registers.sp & 0x000F) + (temp & 0x000F)) > 0x000F);
    SET_FLAG_CARRY(((cpu_registers.sp & 0x00FF) + (temp & 0x00FF)) > 0x00FF);
}

// 0xF9
void cpu_ld_sp_hl() {
    core_advance_cpu_clocks(4);
    cpu_registers.sp = cpu_registers.hl;
    core_advance_cpu_clocks(4);
}

// 0xFA
void cpu_ld_a_nn() {
    core_advance_cpu_clocks(4);
    uint16_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    temp |= memory_bus_read(cpu_registers.pc++) << 8;
    cpu_registers.pc &= 0xFFFF;
    core_advance_cpu_clocks(4);
    cpu_registers.a = memory_bus_read(temp);
    core_advance_cpu_clocks(4);
}

// 0xFB
void cpu_ei() {
    core_advance_cpu_clocks(4);
    interrupt_enable_ime_delay = true;
}

// 0xFE
void cpu_cp_n() {
    core_advance_cpu_clocks(4);
    uint8_t temp = memory_bus_read(cpu_registers.pc++);
    cpu_routine_cp_a_8(temp);
}

// 0xFF
void cpu_rst_38() { cpu_routine_rst_nnnn(0x0038); }
