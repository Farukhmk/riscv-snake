    .section .text.init
    .globl _start
_start:
    lui   sp, %hi(_stack_top)
    addi  sp, sp, %lo(_stack_top)
    # zero .bss
    lui   t0, %hi(_sbss)
    addi  t0, t0, %lo(_sbss)
    lui   t1, %hi(_ebss)
    addi  t1, t1, %lo(_ebss)
1:  bgeu  t0, t1, 2f
    sw    zero, 0(t0)
    addi  t0, t0, 4
    j     1b
2:  call  main
    # main returned: write return value (a0) to EXIT MMIO, then spin
    sw    a0, -8(zero)        # address 0xFFFFFFF8 = EXIT register
3:  j     3b
