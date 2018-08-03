ESPTOOL2 ?= ../esptool2/esptool2

ABOOT_BUILD_BASE ?= build
ABOOT_FW_BASE    ?= firmware

ifndef XTENSA_BINDIR
CC := xtensa-lx106-elf-gcc
LD := xtensa-lx106-elf-gcc
else
CC := $(addprefix $(XTENSA_BINDIR)/,xtensa-lx106-elf-gcc)
LD := $(addprefix $(XTENSA_BINDIR)/,xtensa-lx106-elf-gcc)
endif

ifeq ($(V),1)
Q :=
else
Q := @
endif

CFLAGS    = -Os -O3 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
LDFLAGS   = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static
LD_SCRIPT = eagle.app.v6.ld

E2_OPTS = -quiet -bin -boot0

ifeq ($(ABOOT_BIG_FLASH),1)
	CFLAGS += -DBOOT_BIG_FLASH
endif
ifneq ($(ABOOT_DELAY_MICROS),)
	CFLAGS += -DBOOT_DELAY_MICROS=$(ABOOT_DELAY_MICROS)
endif
ifneq ($(ABOOT_BAUDRATE),)
	CFLAGS += -DBOOT_BAUDRATE=$(ABOOT_BAUDRATE)
endif
ifeq ($(ABOOT_INTEGRATION),1)
	CFLAGS += -DABOOT_INTEGRATION
endif
ifeq ($(ABOOT_RTC_ENABLED),1)
	CFLAGS += -DBOOT_RTC_ENABLED
endif
ifeq ($(ABOOT_CONFIG_CHKSUM),1)
	CFLAGS += -DBOOT_CONFIG_CHKSUM
endif
ifeq ($(ABOOT_GPIO_ENABLED),1)
	CFLAGS += -DBOOT_GPIO_ENABLED
endif
ifeq ($(ABOOT_GPIO_SKIP_ENABLED),1)
	CFLAGS += -DBOOT_GPIO_SKIP_ENABLED
endif
ifneq ($(ABOOT_GPIO_NUMBER),)
	CFLAGS += -DBOOT_GPIO_NUM=$(ABOOT_GPIO_NUMBER)
endif
ifeq ($(ABOOT_IROM_CHKSUM),1)
	CFLAGS += -DBOOT_IROM_CHKSUM
endif
ifneq ($(ABOOT_EXTRA_INCDIR),)
	CFLAGS += $(addprefix -I,$(ABOOT_EXTRA_INCDIR))
endif
CFLAGS += $(addprefix -I,.)

ifeq ($(SPI_SIZE), 256K)
	E2_OPTS += -256
else ifeq ($(SPI_SIZE), 512K)
	E2_OPTS += -512
else ifeq ($(SPI_SIZE), 1M)
	E2_OPTS += -1024
else ifeq ($(SPI_SIZE), 2M)
	E2_OPTS += -2048
else ifeq ($(SPI_SIZE), 4M)
	E2_OPTS += -4096
endif
ifeq ($(SPI_MODE), qio)
	E2_OPTS += -qio
else ifeq ($(SPI_MODE), dio)
	E2_OPTS += -dio
else ifeq ($(SPI_MODE), qout)
	E2_OPTS += -qout
else ifeq ($(SPI_MODE), dout)
	E2_OPTS += -dout
endif
ifeq ($(SPI_SPEED), 20)
	E2_OPTS += -20
else ifeq ($(SPI_SPEED), 26)
	E2_OPTS += -26.7
else ifeq ($(SPI_SPEED), 40)
	E2_OPTS += -40
else ifeq ($(SPI_SPEED), 80)
	E2_OPTS += -80
endif

.SECONDARY:

#all: $(ABOOT_BUILD_BASE) $(ABOOT_FW_BASE) $(ABOOT_FW_BASE)/aboot.bin $(ABOOT_FW_BASE)/testload1.bin $(ABOOT_FW_BASE)/testload2.bin
all: $(ABOOT_BUILD_BASE) $(ABOOT_FW_BASE) $(ABOOT_FW_BASE)/aboot.bin

$(ABOOT_BUILD_BASE):
	mkdir -p $@

$(ABOOT_FW_BASE):
	mkdir -p $@

$(ABOOT_BUILD_BASE)/aboot-stage2a.o: aboot-stage2a.c aboot-private.h aboot.h
	@echo "CC $<"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(ABOOT_BUILD_BASE)/aboot-stage2a.elf: $(ABOOT_BUILD_BASE)/aboot-stage2a.o
	@echo "LD $@"
	$(Q) $(LD) -Taboot-stage2a.ld $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -o $@

$(ABOOT_BUILD_BASE)/aboot-hex2a.h: $(ABOOT_BUILD_BASE)/aboot-stage2a.elf
	@echo "E2 $@"
	$(Q) $(ESPTOOL2) -quiet -header $< $@ .text

$(ABOOT_BUILD_BASE)/aboot.o: aboot.c aboot-private.h aboot.h $(ABOOT_BUILD_BASE)/aboot-hex2a.h
	@echo "CC $<"
	$(Q) $(CC) $(CFLAGS) -I$(ABOOT_BUILD_BASE) -c $< -o $@

$(ABOOT_BUILD_BASE)/%.o: %.c %.h
	@echo "CC $<"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(ABOOT_BUILD_BASE)/%.elf: $(ABOOT_BUILD_BASE)/%.o
	@echo "LD $@"
	$(Q) $(LD) -T$(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -o $@

$(ABOOT_FW_BASE)/%.bin: $(ABOOT_BUILD_BASE)/%.elf
	@echo "E2 $@"
	$(Q) $(ESPTOOL2) $(E2_OPTS) $< $@ .text .rodata

clean:
	@echo "RM $(ABOOT_BUILD_BASE) $(ABOOT_FW_BASE)"
	$(Q) rm -rf $(ABOOT_BUILD_BASE)
	$(Q) rm -rf $(ABOOT_FW_BASE)
