BOOTLOADER_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include ../esp-open-rtos/parameters.mk

all: $(FIRMWARE_DIR)/aboot.bin

aboot/Makefile:
	$(error aboot git submodule is not checkedo out. Try running 'git submodule update --init --recursive')

$(FIRMWARE_DIR)/aboot.bin: $(BUILD_DIR)/aboot.elf $(FIRMWARE_DIR)
	@echo "FW aboot.bin"
	$(Q) $(ESPTOOL) elf2image $(ESPTOOL_ARGS) $< -o $(BUILD_DIR)/
	$(Q) mv $(BUILD_DIR)/0x00000.bin $@

# aboot generates this header using the 'esptool2 -header' option. To try and avoid
# esptool2 as a dependency, we try it here using grep, sed, xxd (all fairly common Unix tools)
$(BUILD_DIR)/aboot-hex2a.h: $(BUILD_DIR)/aboot-stage2a.elf $(BUILD_DIR)
	@echo "Extracting stub image header..."
	$(Q) xtensa-lx106-elf-objcopy $< --only-section .text -Obinary $(BUILD_DIR)/aboot-hex2a.bin
	$(Q) xxd -i $(BUILD_DIR)/aboot-hex2a.bin > $@.in
	$(Q) sed -i "s/unsigned char .\+\[\]/const uint8 _text_data[]/" $@.in
	$(Q) sed -i "s/unsigned int .\+_len/const uint32 _text_len/" $@.in
	$(Q) echo "const uint32 entry_addr = $$(xtensa-lx106-elf-objdump -f $< | grep 'start address' | grep -o '0x.\+');" >> $@.in
	$(Q) echo "const uint32 _text_addr = 0x$$(xtensa-lx106-elf-objdump -h -j .text $< | grep ".text" | grep -o '401.....' | head -n1);" >> $@.in
	$(Q) mv $@.in $@


ABOOT_BUILD_BASE="$(abspath $(BUILD_DIR))"
ABOOT_FW_BASE="$(abspath $(FIRMWARE_DIR))"
MAKE_VARS=ABOOT_EXTRA_INCDIR=$(BOOTLOADER_DIR) ABOOT_BUILD_BASE=$(ABOOT_BUILD_BASE) ABOOT_FW_BASE=$(ABOOT_FW_BASE)

$(BUILD_DIR)/aboot-stage2a.elf: $(BUILD_DIR)
	$(Q) $(MAKE) -C aboot $(ABOOT_BUILD_BASE)/aboot-stage2a.elf $(MAKE_VARS)

$(BUILD_DIR)/aboot.elf: $(BUILD_DIR)/aboot-hex2a.h
	$(Q) $(MAKE) -C aboot $(ABOOT_BUILD_BASE)/aboot.elf $(MAKE_VARS)

$(BUILD_DIR) $(FIRMWARE_DIR):
	mkdir -p $@

flash: $(FIRMWARE_DIR)/aboot.bin
	$(Q) $(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash $(ESPTOOL_ARGS) 0x0 $<

clean:
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(FIRMWARE_DIR)

.PHONY: all
