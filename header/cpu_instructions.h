#pragma once
#include <stdint.h>

struct gb_cpu_instruction {
    const char *disassembly;
    uint8_t operand_length;
    void *execute;
};

extern const gb_cpu_instruction instructions[256];