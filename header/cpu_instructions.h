#pragma once
#include <stdint.h>

#include "cpu.h"

struct gb_cpu_instruction {
    const char *disassembly;
    uint8_t operand_length;
    cpu_execute_op execute;
};

struct gb_cpu_pre_cb_instruction {
    const char *disassembly;
    cpu_execute_op execute;
};

extern const gb_cpu_instruction instructions[256];
extern const gb_cpu_pre_cb_instruction cb_instructions[256];