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

build/battlefield.o: src/battlefield-maps.h src/battlefield-fighter.h src/battlefield-character.h

build/hello.elf: build/start.o build/main.o src/gba.ld
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@

build/battlefield.elf: build/start.o build/battlefield.o src/gba.ld
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@

build/river-trial.o: src/battlefield.c src/battlefield-maps.h src/battlefield-fighter.h src/battlefield-character.h | build
	"$(GBA_CC)" $(CFLAGS) -DSTART_RIVER_TRIAL -c $< -o $@

build/river-trial.elf: build/start.o build/river-trial.o src/gba.ld
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@

build/%.gba: build/%.elf
	"$(OBJCOPY)" -O binary $< $@

test: $(ROM) $(BATTLEFIELD) unit-test
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

build/closeup-art.h: scripts/generate-closeup.py src/battlefield-terrain.def src/battlefield-maps.h src/battlefield-fighter.h | build
	$(PYTHON) $< > $@

build/battlefield.o build/river-trial.o: build/closeup-art.h

.PHONY: preview-battle-audio
preview-battle-audio: build/closeup-art.h build/start.o
	$(PYTHON) scripts/convert-battle-wav.py "$(BATTLE_WAV)" build/preview-battle.pcm 30
	$(if $(strip $(MAP_WAV)),$(PYTHON) scripts/convert-battle-wav.py "$(MAP_WAV)" build/preview-map.pcm 30)
	"$(GBA_CC)" $(CFLAGS) -DSTART_RIVER_TRIAL -DLOCAL_BATTLE_AUDIO $(if $(strip $(MAP_WAV)),-DLOCAL_MAP_AUDIO) -c src/battlefield.c -o build/preview-battle.o
	"$(GBA_CC)" $(ARCH) -nostdlib -Wl,--build-id=none,-T,src/gba.ld -o build/preview-battle.elf build/start.o build/preview-battle.o
	"$(OBJCOPY)" -O binary build/preview-battle.elf build/preview-local-only.gba

build/item-data.h: scripts/generate-items.py data/items-srd-5.2.1.json | build
	$(PYTHON) $^ > $@

build/battlefield.o build/river-trial.o preview-battle-audio: build/item-data.h

.PHONY: unit-test smoke-preview
unit-test: build/battlefield-rules-test
	./build/battlefield-rules-test

build/battlefield-rules-test: tests/battlefield-rules.c src/battlefield.c src/battlefield-character.h src/battlefield-maps.h src/battlefield-fighter.h src/battlefield-terrain.def build/closeup-art.h build/item-data.h
	$(CC) -std=c11 -O2 -Wall -Wextra -Werror -o $@ $<

smoke-preview:
	$(SMOKE) scripts/smoke-battlefield.py --preview "$(MGBA_CORE)" build/preview-local-only.gba build/preview-check.png
