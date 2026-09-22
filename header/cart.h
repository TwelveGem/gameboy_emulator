#pragma once

#include <stdint.h>

const int MAX_CART_SIZE = 1024 * 1024;
extern uint8_t cartridge_data[MAX_CART_SIZE];
extern bool cartridge_loaded;

struct cart_header_struct {
    uint8_t entry_point[4];
    uint8_t nintendo_logo[48];
    uint8_t title[15];
    uint8_t cgb_flag;
    uint8_t new_licensee_code[2];
    uint8_t sgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t ram_size;
    uint8_t destination_code;
    uint8_t old_licensee_code;
    uint8_t mask_rom_version_number;
    uint8_t header_checksum;
    uint8_t global_checksum_hi;
    uint8_t global_checksum_lo;
};

extern cart_header_struct *cartridge_header;

bool cart_open_file();
void cart_print_info();
bool cart_load(const char *filename);
