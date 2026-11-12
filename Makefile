.DELETE_ON_ERROR:

TOOLCHAIN := .toolchain/noble-amd64/usr
LOCAL_BIN := $(if $(wildcard $(TOOLCHAIN)/bin/arm-none-eabi-gcc),$(TOOLCHAIN)/bin/)
CROSS_COMPILE ?= $(LOCAL_BIN)arm-none-eabi-
GBA_CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
PYTHON ?= python3
MGBA ?= mgba
LOCAL_CORE := $(TOOLCHAIN)/lib/x86_64-linux-gnu/libretro/mgba_libretro.so
MGBA_CORE ?= $(if $(wildcard $(LOCAL_CORE)),$(LOCAL_CORE),/usr/lib/$(shell uname -m)-linux-gnu/libretro/mgba_libretro.so)

ARCH := -mcpu=arm7tdmi -marm
CFLAGS := $(ARCH) -std=c11 -Os -Wall -Wextra -Werror -ffreestanding -fno-unwind-tables -fno-asynchronous-unwind-tables
ROM := build/hello.gba
BATTLEFIELD := build/battlefield.gba
SMOKE := LD_LIBRARY_PATH="$(TOOLCHAIN)/lib/x86_64-linux-gnu$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}" timeout 15s $(PYTHON)

.PHONY: all test smoke smoke-battlefield run battlefield run-battlefield clean
all: $(ROM) $(BATTLEFIELD)

battlefield: $(BATTLEFIELD)

build:
	mkdir -p $@

build/start.o: src/start.S | build
	"$(GBA_CC)" $(ARCH) -c $< -o $@

build/%.o: src/%.c | build
	"$(GBA_CC)" $(CFLAGS) -c $< -o $@

build/battlefield.o: src/battlefield-maps.h src/battlefield-fighter.h

build/hello.elf: build/start.o build/main.o src/gba.ld
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@

build/battlefield.elf: build/start.o build/battlefield.o src/gba.ld
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@

build/river-trial.o: src/battlefield.c src/battlefield-maps.h src/battlefield-fighter.h | build
	"$(GBA_CC)" $(CFLAGS) -DSTART_RIVER_TRIAL -c $< -o $@

build/river-trial.elf: build/start.o build/river-trial.o src/gba.ld
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@

build/%.gba: build/%.elf
	"$(OBJCOPY)" -O binary $< $@

test: $(ROM) $(BATTLEFIELD)
	$(PYTHON) scripts/check-rom.py $(ROM)
	$(MAKE) smoke smoke-battlefield

smoke: $(ROM)
	$(SMOKE) scripts/smoke.py "$(MGBA_CORE)" $(ROM) build/hello.png

smoke-battlefield: $(BATTLEFIELD) build/river-trial.gba
	$(SMOKE) scripts/smoke-battlefield.py "$(MGBA_CORE)" $(BATTLEFIELD) build/river-trial.gba build/battlefield.png

run: $(ROM)
	"$(MGBA)" $(ROM)

run-battlefield: $(BATTLEFIELD)
	"$(MGBA)" $<

clean:
	rm -rf build
