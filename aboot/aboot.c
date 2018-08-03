
#include "aboot-private.h"
#include <aboot-hex2a.h>

static uint8 write_conf(void) {
	uint8 buffer[SECTOR_SIZE];
	uint8 rom_to_boot;

	aboot_config *romconf = (aboot_config*)buffer;

	// read boot config
	SPIRead(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);
	rom_to_boot = romconf->current_rom;

	ets_printf("Writing default boot config.\r\n");
	ets_memset(romconf, 0x00, sizeof(aboot_config));
	romconf->magic = BOOT_CONFIG_MAGIC;
	romconf->version = BOOT_CONFIG_VERSION;
	romconf->count = MAX_ROMS;
	romconf->roms[0] = ROM_ADDR_1;
	romconf->roms[1] = ROM_ADDR_2;
	romconf->current_rom = 0;
	SPIEraseSector(BOOT_CONFIG_SECTOR);
	SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);

	return rom_to_boot;
}

static uint32 get_run_address() {
	uint8 buffer[BUFFER_SIZE];
	uint32 runaddr;

	rom_header_new *header = (rom_header_new*)buffer;

	if (SPIRead(ROM_ADDR_1, header, sizeof(rom_header_new)) != 0) {
		return 0;
	}

	ets_printf("Header size: %d\r\n", header->len + sizeof(rom_header_new));
	// new type, has extra header and irom section first
	runaddr = ROM_ADDR_1 + header->len + sizeof(rom_header_new);

	return runaddr;
}

void clear_rom(uint32 pos) {
	uint32 end = pos + ROM_SIZE;
	ets_printf("Clear rom at position %x\n", pos);
	for(;pos < end; pos += SECTOR_SIZE) {
		if (SPIEraseSector(pos / SECTOR_SIZE) != 0) {
			ets_printf("SPIEraseSector err at position %x sector %d\n", pos, pos / SECTOR_SIZE);
			return;
		}
	}
}

void copy_rom() {
	uint8 buffer[BUFFER_SIZE];
	uint32 pos = 0;
	uint32 end = ROM_SIZE;
	for(; pos < end; pos += BUFFER_SIZE) {
		ets_printf("Write at %d copy of %d\n", ROM_ADDR_1 + pos, ROM_ADDR_2 + pos);
		if (SPIRead(ROM_ADDR_2 + pos, buffer, BUFFER_SIZE) != 0) {
			ets_printf("read err %d\n", ROM_ADDR_2 + pos);
			return;
		}
		SPIWrite(ROM_ADDR_1 + pos, buffer, BUFFER_SIZE);
	}
}

uint32 NOINLINE find_image(void) {
	uint32 runAddr;
	uint8 rom_to_boot;

	rom_to_boot = write_conf();
	if (rom_to_boot > 0) {
		ets_printf("We should copy the new Rom from %x to %x.\r\n", ROM_ADDR_2, ROM_ADDR_1);
		clear_rom(ROM_ADDR_1);
		copy_rom();
		clear_rom(ROM_ADDR_2);
	}

	runAddr = get_run_address();
	ets_printf("Booting at ram %d.\r\n", runAddr);
	// copy the loader to top of iram
	ets_memcpy((void*)_text_addr, _text_data, _text_len);

	return runAddr;
}

// assembler stub uses no stack space
// works with gcc
void call_user_start(void) {
	__asm volatile (
		"mov a15, a0\n"          // store return addr, hope nobody wanted a15!
		"call0 find_image\n"     // find a good rom to boot
		"mov a0, a15\n"          // restore return addr
		"bnez a2, 1f\n"          // ?success
		"ret\n"                  // no, return
		"1:\n"                   // yes...
		"movi a3, entry_addr\n"  // get pointer to entry_addr
		"l32i a3, a3, 0\n"       // get value of entry_addr
		"jx a3\n"                // now jump to it
	);
}
