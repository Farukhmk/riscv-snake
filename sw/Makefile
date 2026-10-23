# Auto-pick toolchain prefix
PREFIX ?= $(shell command -v riscv-none-elf-gcc >/dev/null 2>&1 && echo riscv-none-elf- || echo riscv64-unknown-elf-)
CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE    = $(PREFIX)size

B = build
SRC ?= main.c
CFLAGS  = -march=rv32i -mabi=ilp32 -O2 -ffreestanding -nostdlib -fno-pic \
          -fno-jump-tables -mno-relax -msmall-data-limit=0 -Wall
LDFLAGS = -T linker.ld -nostdlib -Wl,--no-warn-rwx-segments -Wl,-Map=$(B)/prog.map

RTL = $(filter-out ../tb_%.sv ../%_tb.sv,$(wildcard ../*.sv))

all: $(B)/program.hex $(B)/data_b0.hex

$(B):
	mkdir -p $(B)

$(B)/prog.elf: startup.S $(SRC) linker.ld | $(B)
	$(CC) $(CFLAGS) $(LDFLAGS) startup.S $(SRC) -o $@ -lgcc
	$(SIZE) $@

$(B)/text.bin: $(B)/prog.elf
	$(OBJCOPY) -O binary -j .text $< $@
$(B)/data.bin: $(B)/prog.elf
	$(OBJCOPY) -O binary -j .rodata -j .data $< $@

$(B)/program.hex: $(B)/text.bin
	python3 bin2mem.py words $< $@
$(B)/data_b0.hex: $(B)/data.bin
	python3 bin2mem.py banks $< $(B)/data_b

dump: $(B)/prog.elf
	$(OBJDUMP) -d $< > $(B)/prog.dis

# fail if any non-RV32I instruction slipped in (mul/div/compressed)
check: dump
	@! grep -E -w "mul|mulh|mulhu|mulhsu|div|divu|rem|remu|c\.[a-z]+|__mulsi3|__divsi3|__udivsi3|__modsi3|__umodsi3" $(B)/prog.dis \
	  && echo "OK: base RV32I only"

sim: all
	verilator --binary --timing -Wno-fatal -Mdir $(B)/obj_dir --top-module tb_c_program \
	  $(RTL) tb_c_program.sv
	cd $(B) && ./obj_dir/Vtb_c_program

clean:
	rm -rf $(B)
.PHONY: all dump check sim clean
