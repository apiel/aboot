#ifndef __ABOOT_H__
#define __ABOOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <c_types.h>

#define BOOT_CONFIG_MAGIC 0xe1
#define BOOT_CONFIG_VERSION 0x01
#define SECTOR_SIZE 0x1000
#define BUFFER_SIZE 0x100
#define MAX_ROMS 2
#define BOOT_CONFIG_SECTOR 1
#define ROM_SIZE 0x55000 // 348160 bytes
#define ROM_ADDR_1 0x2000
#define ROM_ADDR_2 ROM_SIZE + ROM_ADDR_1
#define YES 1
#define NO 0

// For the moment I guess I must keep the same structure at rboot for esp-open-rtos
// Let see later if this can change somehow
typedef struct {
	uint8 magic;           ///< Our magic, identifies rBoot configuration - should be BOOT_CONFIG_MAGIC
	uint8 version;         ///< Version of configuration structure - should be BOOT_CONFIG_VERSION
	uint8 mode;            ///< Boot loader mode (MODE_STANDARD | MODE_GPIO_ROM | MODE_GPIO_SKIP)
	uint8 current_rom;     ///< Currently selected ROM (will be used for next standard boot)
	uint8 gpio_rom;        ///< ROM to use for GPIO boot (hardware switch) with mode set to MODE_GPIO_ROM
	uint8 count;           ///< Quantity of ROMs available to boot
	uint8 unused[2];       ///< Padding (not used)
	uint32 roms[MAX_ROMS]; ///< Flash addresses of each ROM
} aboot_config;

#ifdef __cplusplus
}
#endif

#endif
