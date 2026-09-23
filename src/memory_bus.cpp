#include "memory_bus.h"
#include "cart.h"
#include "cart_type.h"
#include "interrupts.h"
#include "timer.h"
#include <cassert>
#include <stdio.h>

uint8_t memory[MEMORY_SIZE];
uint8_t eram[ERAM_SIZE];

uint16_t rom_bank_number = 1;
uint16_t ram_bank_number = 0;
bool ram_enabled = false;
bool rom_ram_mode_select = false;

static uint16_t rom_bank_count() {
    const uint32_t rom_size = 32 * (1 << cartridge_header->rom_size); // 32k << N
    return rom_size / 16;                                             // 16k per ROM bank
}

// READING FROM ADDRESSES 0x4000-0x7FFF
uint8_t read_rom_banks(const uint16_t addr) {
    const cart_type_info &cart_info = cart_type_data[cartridge_header->cartridge_type];

    if (cart_info.type == CART_TYPE::NO_MBC) {
        return cartridge_data[addr]; // For no MBC cartridges

    } else if (cart_info.type == CART_TYPE::MBC1 || cart_info.type == CART_TYPE::MBC2) {
        const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank
        const uint16_t rom_bank = rom_bank_number % rom_bank_count();
        const uint32_t offset = ROM_BANK_SIZE * rom_bank;

        return cartridge_data[offset + (addr - 0x4000)];

    } else if (cart_info.type == CART_TYPE::MBC3) {
        const uint16_t ROM_BANK_SIZE = 0x4000;
        uint8_t rom_bank = rom_bank_number > 0 ? rom_bank_number : 1;
        const uint32_t offset = ROM_BANK_SIZE * (rom_bank - 1);

        return cartridge_data[offset + addr];
    }

    return 0xFF;
}

// READING FROM ADDRESSES 0xA000-0xBFFF
uint8_t read_external_ram(const uint16_t addr) {
    const cart_type_info &cart_info = cart_type_data[cartridge_header->cartridge_type];

    if (cart_info.type == CART_TYPE::NO_MBC) {
        const uint16_t eram_address = (addr - 0xA000);

        return eram[eram_address];

    } else if (cart_info.type == CART_TYPE::MBC1 && ram_enabled) {
        const uint16_t bank_offset = ram_bank_number * 0x2000;
        const uint16_t eram_address = (addr - 0xA000) + bank_offset;

        return eram[eram_address];

    } else if (cart_info.type == CART_TYPE::MBC2 && ram_enabled) {
        const uint16_t offset = (addr - 0xA000) % 0x0200;

        return 0xF0 | (0x0F & eram[offset]);
    }

    return 0xFF;
}

uint8_t memory_bus_read(const uint16_t addr) {
    if (addr <= 0x3FFF) { // Read from ROM bank 00
        return cartridge_data[addr];
    } else if (addr <= 0x7FFF) { // Read from ROM bank 01-NN
        return read_rom_banks(addr);
    } else if (addr <= 0x9FFF) { // VRAM
        return memory[addr];
    } else if (addr <= 0xBFFF) { // External RAM
        return read_external_ram(addr);
    } else if (addr <= 0xCFFF) { // Work RAM 1
        return memory[addr];
    } else if (addr <= 0xDFFF) { // Work RAM 2
        return memory[addr];
    } else if (addr <= 0xFDFF) { // Echo RAM (Mirror of C000-DDFF)
        return memory[addr - 0x2000];
    } else if (addr <= 0xFE9F) { // Object Attribute Memory (sprite attribute table)
        // TODO: If PPU mode == 2, return 0xFF
        return memory[addr];
    } else if (addr <= 0xFEFF) { // Unused
        return 0x00;
    } else if (addr <= 0xFF7F) { // I/O register
        return memory[addr];
    } else if (addr <= 0xFFFE) { // Interrupt Enable register
        return memory[addr];
    }

    return 0xFF;
}

void register_write_mbc1(const uint16_t addr, const uint8_t value) {
    assert(addr >= 0x0000 && addr <= 0x7FFF);

    if (addr >= 0x0000 && addr <= 0x1FFF) { // RAM Enable
        ram_enabled = (value & 0x0F) == 0x0A;
    }

    if (addr >= 0x2000 && addr <= 0x3FFF) {
        uint8_t bank_low = value & 0x1F; // 5-bit register
        if (bank_low == 0) {             // $00 becomes $01 before bits 5-6 are attached, so
            bank_low = 1;                // banks $00/$20/$40/$60 are unreachable at 4000-7FFF
        }
        rom_bank_number = (rom_bank_number & 0x60) | bank_low; // bits 5-6 stay
    }

    if (addr >= 0x4000 && addr <= 0x5FFF) {
        if (rom_ram_mode_select) {
            ram_bank_number = value & 0x03; // 2-bit register
        } else {
            rom_bank_number = (rom_bank_number & 0x1F) | ((value & 0x03) << 5);
        }
    }

    if (addr >= 0x6000 && addr <= 0x7FFF) {
        rom_ram_mode_select = value & 0x01; // 1-bit register
    }
}

void register_write_mbc2(const uint16_t addr, const uint8_t value) {
    assert(addr >= 0x0000 && addr <= 0x3FFF);

    if ((0x0100 & addr) == 0) {               // When bit-8 is clear
        ram_enabled = (value & 0x0F) == 0x0A; // enable/disable ram
    } else {                                  // otherwise
        rom_bank_number = value & 0x0F;       // set the rom_bank_number
        if (rom_bank_number == 0) {           //     based on a 4-bit register
            rom_bank_number = 1;              // $00 becomes $01
        }
    }
}

// ADDR must be between 0x8000 and 0x9FFF
void write_vram(const uint16_t addr, const uint8_t value) {
    assert(addr >= 0x8000 && addr <= 0x9FFF);

    // TODO: If PPU is in mode 3, the CPU cannot access VRAM
    memory[addr] = value;
}

void write_external_ram(const uint16_t addr, const uint8_t value) {
    assert(addr >= 0xA000 && addr <= 0xBFFF);

    const cart_type_info &cart_info = cart_type_data[cartridge_header->cartridge_type];

    if (cart_info.type == CART_TYPE::NO_MBC && ram_enabled) {
        eram[addr - 0xA000] = value;
    } else if (cart_info.type == CART_TYPE::MBC1) {
        const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per ROM bank
        const uint32_t offset = RAM_BANK_SIZE * ram_bank_number;
        const uint16_t eram_address = offset * (addr - 0xA000);
        eram[eram_address] = value;
    } else if (cart_info.type == CART_TYPE::MBC2 && ram_enabled) {
        const uint16_t offset = addr & 0x01FF;
        eram[offset] = value & 0x0F;
    }
}

void write_io_registers(const uint16_t addr, const uint8_t value) {
    assert(addr >= 0xFF00 && addr <= 0xFF7F);

    if (addr == 0xFF02 && value == 0x81) { // Serial print
        char c = memory[0xFF01];
        printf("%c", c);
    } else if (addr == 0xFF04) { // Timer DIV
        timer_on_div_write(value);
    } else if (addr == 0xFF07) { // Timer control
        memory[addr] = value & 0x07;
    } else if (addr == 0xFF0F) { // Interrupt flag
        interrupt_flag_write(value);
    } else if (addr == 0xFF46) {
        uint16_t source_addr = value * 0x100;
        const uint16_t dest = 0xFE00;
        for (uint8_t i = 0; i < 160; i++) {
            memory[dest + i] = memory_bus_read(source_addr + i);
        }
    } else {
        memory[addr] = value;
    }
}

void memory_bus_write(const uint16_t addr, const uint8_t value) {
    const cart_type_info &cart_info = cart_type_data[cartridge_header->cartridge_type];
    if (addr < 0x8000) {
        switch (cart_info.type) {
        case CART_TYPE::MBC1:
            register_write_mbc1(addr, value);
            break;
        case CART_TYPE::MBC2:
            register_write_mbc2(addr, value);
            break;
        default:
            assert(false);
        }
    }

    if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
        write_vram(addr, value);
    }
    if (addr >= 0xA000 && addr <= 0xBFFF) { // ERAM
        write_external_ram(addr, value);
    }
    if (addr >= 0xC000 && addr <= 0xCFFF) { // WRAM 1
        memory[addr] = value;
    }
    if (addr >= 0xD000 && addr <= 0xDFFF) { // WRAM 2
        memory[addr] = value;
    }
    if (addr >= 0xE000 && addr <= 0xFDFF) { // Echo RAM
        memory[addr - 0x2000] = value;
    }
    if (addr >= 0xFE00 && addr <= 0xFE9F) { // OAM Data
        // TODO: If PPU is in mode 2 or 3, the CPU cannot access OAM
        memory[addr] = value;
    }
    if (addr >= 0xFEA0 && addr <= 0xFEFF) { // Unused memory
        // All writes here are ignored
    }
    if (addr >= 0xFF00 && addr <= 0xFF7F) { // I/O registers
        write_io_registers(addr, value);
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE) { // HRAM
        memory[addr] = value;
    }
    if (addr == 0xFFFF) { // Interrupt Enable
        interrupt_enable_write(value);
    }
}
