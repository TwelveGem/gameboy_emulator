#include "cpu.h"
#include "cart.h"
#include "cpu_instructions.h"
#include <stdio.h>

gb_cpu_registers cpu_registers;
uint8_t cpu_current_op_code = 0;
uint32_t cpu_instruction_counter = 0;
void *cpu_current_instruction_execute = nullptr;

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
    cpu_current_op_code = cartridge_data[cpu_registers.pc++];
    const gb_cpu_instruction &instruction = instructions[cpu_current_op_code];
    cpu_current_instruction_execute = instruction.execute;
}

bool cpu_execute() {
    if (!cpu_current_instruction_execute) {
        const gb_cpu_instruction &instruction =
            instructions[cpu_current_op_code];
        const uint8_t pchi = ((cpu_registers.pc - 1) & 0xFF00) >> 8;
        const uint8_t pclo = ((cpu_registers.pc - 1) & 0xFF);
        printf("Unknown instruction at: %.2X%.2X (%s), count %i\n", pchi, pclo,
               instruction.disassembly, cpu_instruction_counter);
        return false;
    }

    ((cpu_execute_op)cpu_current_instruction_execute)();

    return true;
}
