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

#define cpu_routine_add_a_8(reg8)                                                                                      \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        uint32_t macro_temp = cpu_registers.a;                                                                         \
        SET_FLAG_HALF_CARRY(((macro_temp & 0xF) + ((uint32_t)reg8 & 0xF)) > 0xF);                                      \
        cpu_registers.a += reg8;                                                                                       \
        SET_FLAG_ZERO(cpu_registers.a == 0);                                                                           \
        SET_FLAG_CARRY(macro_temp > cpu_registers.a);                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_add_hl_16(reg16)                                                                                   \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        uint32_t macro_temp = cpu_registers.hl + reg16;                                                                \
        SET_FLAG_CARRY(macro_temp > 0xFFFF);                                                                           \
        bool hc = ((cpu_registers.hl & 0x0FFF) + (reg16 & 0x0FFF)) > 0x0FFF;                                           \
        SET_FLAG_HALF_CARRY(hc);                                                                                       \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.hl = macro_temp & 0xFFFF;                                                                        \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_adc_a_8(reg8)                                                                                      \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        uint8_t carry = GET_FLAG_CARRY;                                                                                \
        uint32_t macro_temp = cpu_registers.a + reg8 + carry;                                                          \
        bool hc = (((cpu_registers.a & 0xF) + (reg8 & 0xF)) + carry) > 0xF;                                            \
        SET_FLAG_HALF_CARRY(hc);                                                                                       \
        SET_FLAG_CARRY(macro_temp > 0xFF);                                                                             \
        macro_temp &= 0xFF;                                                                                            \
        cpu_registers.a = macro_temp;                                                                                  \
        SET_FLAG_ZERO(macro_temp == 0);                                                                                \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_sub_a_8(reg8)                                                                                      \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(1);                                                                                          \
        SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (reg8 & 0xF));                                                   \
        SET_FLAG_CARRY((uint32_t)cpu_registers.a < (uint32_t)reg8);                                                    \
        cpu_registers.a -= reg8;                                                                                       \
        SET_FLAG_ZERO(cpu_registers.a == 0);                                                                           \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_sbc_a_8(reg8)                                                                                      \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(1);                                                                                          \
        uint8_t carry = GET_FLAG_CARRY;                                                                                \
        SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (reg8 & 0xF) + carry);                                           \
        SET_FLAG_CARRY((uint32_t)cpu_registers.a < (uint32_t)reg8 + carry);                                            \
        cpu_registers.a = cpu_registers.a - reg8 - carry;                                                              \
        SET_FLAG_ZERO(cpu_registers.a == 0);                                                                           \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_and_a_8(reg8)                                                                                      \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(1);                                                                                        \
        SET_FLAG_CARRY(0);                                                                                             \
        cpu_registers.a &= reg8;                                                                                       \
        SET_FLAG_ZERO(cpu_registers.a == 0);                                                                           \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_xor_a_8(reg8)                                                                                      \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY(0);                                                                                             \
        cpu_registers.a ^= reg8;                                                                                       \
        SET_FLAG_ZERO(cpu_registers.a == 0);                                                                           \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_or_a_8(reg8)                                                                                       \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY(0);                                                                                             \
        cpu_registers.a |= reg8;                                                                                       \
        SET_FLAG_ZERO(cpu_registers.a == 0);                                                                           \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_cp_a_8(reg8)                                                                                       \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(1);                                                                                          \
        SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (reg8 & 0xF));                                                   \
        SET_FLAG_CARRY((uint32_t)cpu_registers.a < (uint32_t)reg8);                                                    \
        SET_FLAG_ZERO(cpu_registers.a == reg8);                                                                        \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_rst_nnnn(addr)                                                                                     \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.sp--;                                                                                            \
        cpu_registers.sp &= 0xFFFF;                                                                                    \
        const int8_t pchi = (cpu_registers.pc & 0xFF00) >> 8;                                                          \
        core_advance_cpu_clocks(4);                                                                                    \
        memory_bus_write(cpu_registers.sp, pchi);                                                                      \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.sp--;                                                                                            \
        cpu_registers.sp &= 0xFFFF;                                                                                    \
        const uint8_t pclo = cpu_registers.pc & 0xFF;                                                                  \
        memory_bus_write(cpu_registers.sp, pclo);                                                                      \
        cpu_registers.pc = addr;                                                                                       \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_push_16(reg_hi, reg_low)                                                                           \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.sp--;                                                                                            \
        cpu_registers.sp &= 0xFFFF;                                                                                    \
        core_advance_cpu_clocks(4);                                                                                    \
        memory_bus_write(cpu_registers.sp, reg_hi);                                                                    \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.sp--;                                                                                            \
        cpu_registers.sp &= 0xFFFF;                                                                                    \
        memory_bus_write(cpu_registers.sp, reg_low);                                                                   \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_pop_16(reg_hi, reg_low)                                                                            \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        reg_low = memory_bus_read(cpu_registers.sp++);                                                                 \
        cpu_registers.sp &= 0xFFFF;                                                                                    \
        core_advance_cpu_clocks(4);                                                                                    \
        reg_hi = memory_bus_read(cpu_registers.sp++);                                                                  \
        cpu_registers.sp &= 0xFFFF;                                                                                    \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_call_conditional_nnnn(cond)                                                                        \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        if (cond) {                                                                                                    \
            uint32_t macro_temp = memory_bus_read(cpu_registers.pc++);                                                 \
            core_advance_cpu_clocks(4);                                                                                \
            macro_temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8;                                        \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.sp--;                                                                                        \
            cpu_registers.sp &= 0xFFFF;                                                                                \
            const uint8_t pchi = (cpu_registers.pc & 0xFF00) >> 8;                                                     \
            core_advance_cpu_clocks(4);                                                                                \
            memory_bus_write(cpu_registers.sp, pchi);                                                                  \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.sp--;                                                                                        \
            cpu_registers.sp &= 0xFFFF;                                                                                \
            const uint8_t pclo = (cpu_registers.pc & 0xFF);                                                            \
            memory_bus_write(cpu_registers.sp, pclo);                                                                  \
            cpu_registers.pc = macro_temp;                                                                             \
            core_advance_cpu_clocks(4);                                                                                \
        } else {                                                                                                       \
            cpu_registers.pc++;                                                                                        \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.pc++;                                                                                        \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.pc &= 0xFFFF;                                                                                \
        }                                                                                                              \
    }

#define cpu_routine_return_conditional(cond)                                                                           \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        if (cond) {                                                                                                    \
            uint32_t macro_temp = memory_bus_read(cpu_registers.sp++);                                                 \
            cpu_registers.sp &= 0xFFFF;                                                                                \
            core_advance_cpu_clocks(4);                                                                                \
            macro_temp |= memory_bus_read(cpu_registers.sp++) << 8;                                                    \
            cpu_registers.sp &= 0xFFFF;                                                                                \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.pc = macro_temp;                                                                             \
            core_advance_cpu_clocks(4);                                                                                \
        }                                                                                                              \
        core_advance_cpu_clocks(4);                                                                                    \
    }

// Unconditional RET / RETI is 16 clocks, not the 20 of a taken conditional return
#define cpu_routine_return()                                                                                           \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        uint32_t macro_temp = memory_bus_read(cpu_registers.sp++);                                                     \
        core_advance_cpu_clocks(4);                                                                                    \
        macro_temp |= memory_bus_read(cpu_registers.sp++) << 8;                                                        \
        core_advance_cpu_clocks(4);                                                                                    \
        cpu_registers.pc = macro_temp;                                                                                 \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_jump_conditional_nnnn(cond)                                                                        \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        if (cond) {                                                                                                    \
            uint32_t macro_temp = memory_bus_read(cpu_registers.pc++);                                                 \
            core_advance_cpu_clocks(4);                                                                                \
            macro_temp |= memory_bus_read(cpu_registers.pc++) << 8;                                                    \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.pc = macro_temp;                                                                             \
            core_advance_cpu_clocks(4);                                                                                \
        } else {                                                                                                       \
            cpu_registers.pc++;                                                                                        \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.pc++;                                                                                        \
            core_advance_cpu_clocks(4);                                                                                \
        }                                                                                                              \
    }

#define cpu_routine_jr_conditional_n(cond)                                                                             \
    {                                                                                                                  \
        core_advance_cpu_clocks(4);                                                                                    \
        if (cond) {                                                                                                    \
            uint32_t macro_temp = memory_bus_read(cpu_registers.pc++);                                                 \
            core_advance_cpu_clocks(4);                                                                                \
            cpu_registers.pc += (int8_t)macro_temp & 0xFFFF;                                                           \
            core_advance_cpu_clocks(4);                                                                                \
        } else {                                                                                                       \
            cpu_registers.pc++;                                                                                        \
            core_advance_cpu_clocks(4);                                                                                \
        }                                                                                                              \
    }

#define cpu_routine_rlc_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY((reg8 & 0x80) != 0);                                                                            \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = (reg8 << 1) | GET_FLAG_CARRY;                                                                           \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
    }

#define cpu_routine_rrc_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY(reg8 & 0x01);                                                                                   \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = (reg8 >> 1) | GET_FLAG_CARRY * 0x80;                                                                    \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
    }

#define cpu_routine_rl_8(reg8)                                                                                         \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        bool carry = GET_FLAG_CARRY;                                                                                   \
        SET_FLAG_CARRY((reg8 & 0x80) != 0);                                                                            \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = (reg8 << 1) | carry;                                                                                    \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
    }

#define cpu_routine_rr_8(reg8)                                                                                         \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        bool carry = GET_FLAG_CARRY;                                                                                   \
        SET_FLAG_CARRY(reg8 & 0x01);                                                                                   \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = (reg8 >> 1) | carry * 0x80;                                                                             \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
    }

#define cpu_routine_sla_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY((reg8 & 0x80) != 0);                                                                            \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = reg8 << 1;                                                                                              \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
    }

#define cpu_routine_sra_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY((reg8 & 0x01) != 0);                                                                            \
        core_advance_cpu_clocks(4);                                                                                    \
        reg8 = (reg8 & 0x80) | reg8 >> 1;                                                                              \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
    }

#define cpu_routine_swap_8(reg8)                                                                                       \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY(0);                                                                                             \
        reg8 = (reg8 >> 4) | ((reg8 & 0x0F) << 4);                                                                     \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
        core_advance_cpu_clocks(4);                                                                                    \
    }

#define cpu_routine_srl_8(reg8)                                                                                        \
    {                                                                                                                  \
        SET_FLAG_SUBTRACT(0);                                                                                          \
        SET_FLAG_HALF_CARRY(0);                                                                                        \
        SET_FLAG_CARRY(reg8 & 0x01);                                                                                   \
        reg8 = reg8 >> 1;                                                                                              \
        SET_FLAG_ZERO(reg8 == 0);                                                                                      \
        core_advance_cpu_clocks(4);                                                                                    \
    }
