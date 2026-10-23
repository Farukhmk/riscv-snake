/* Harvard layout matching rv32i_core_pipelined_full:
 *   IMEM: instruction BRAM, 4096 words (16 KB), PC reset vector 0x0, read-only
 *   DMEM: data BRAM, 16 KB. data_mem_bram only decodes addr[13:2], so the
 *         0x10000000 base aliases to byte offset 0 of the BRAM.
 * .rodata/.data must live in DMEM because the CPU cannot load from IMEM. */
ENTRY(_start)
MEMORY
{
    IMEM (rx) : ORIGIN = 0x00000000, LENGTH = 16K
    DMEM (rw) : ORIGIN = 0x10000000, LENGTH = 16K
}
SECTIONS
{
    .text : { *(.text.init) *(.text .text.*) } > IMEM
    .rodata : { *(.rodata .rodata.* .srodata .srodata.*) . = ALIGN(4); } > DMEM
    .data   : { *(.data .data.* .sdata .sdata.*) . = ALIGN(4); } > DMEM
    .bss (NOLOAD) : {
        _sbss = .;
        *(.sbss .sbss.* .bss .bss.* COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > DMEM
    _stack_top = ORIGIN(DMEM) + LENGTH(DMEM);
    /DISCARD/ : { *(.comment) *(.riscv.attributes) *(.eh_frame*) }
}
