#pragma once

#define cpu_routine_ld_8(reg8)                                                                                         \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = memory_bus_read(cpu_registers.pc++);                                                                    \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_ld_16(reg_hi, reg_low)                                                                             \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        reg_low = memory_bus_read(cpu_registers.pc++);                                                                 \
        core_advance_cpu_clocks(4);                                                                                    \
        reg_hi = memory_bus_read(cpu_registers.pc++);                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_inc_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        reg8++;                                                                                                        \
        SET_FLAG_HALF_CARRY((reg8 & 0xF) == 0x0);                                                                      \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_dec_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(1);                                                                                          \
        SET_FLAG_HALF_CARRY((reg8 & 0xF) == 0x0);                                                                      \
        reg8--;                                                                                                        \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_inc_16(reg16)                                                                                      \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        reg16 = reg16 + 1;                                                                                             \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_dec_16(reg16)                                                                                      \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        reg16 = reg16 - 1;                                                                                             \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_ld_ptr8(reg16, reg8)                                                                               \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        memory_bus_write(reg16, reg8);                                                                                 \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_ld_ptr_16(reg8, reg16)                                                                             \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = memory_bus_read(reg16);                                                                                 \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_add_hl_16(reg16)                                                                                   \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        uint32_t temp = cpu_registers.hl + reg16;                                                                      \
        SET_FLAG_CARRY(temp > 0xFFFF);                                                                                 \
        bool hc = ((cpu_registers.hl & 0x0FFF) + (reg16 & 0x0FFF)) > 0x0FFF;                                           \
        SET_FLAG_HALF_CARRY(hc);                                                                                       \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.hl = temp & 0xFFFF;                                                                              \
        core_advance_cpu_clocks(4);                                                                                    \
    }