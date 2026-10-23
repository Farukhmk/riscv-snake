# 🐍 RV32I Snake Benchmark

A self-playing **Snake game written in C and executed on a custom 32-bit RV32I RISC-V processor**.

The project uses Snake as a real application workload to evaluate the behavior and performance of a custom 5-stage pipelined CPU.

## Processor

- 32-bit **RV32I** ISA
- 5-stage pipeline
- SystemVerilog RTL
- Instruction and data memory using BRAM-friendly architecture
- Forwarding and hazard detection
- Load-use hazard handling
- Branch/jump handling
- GPIO interface

## Software Flow

```text
C Program
   ↓
RISC-V GCC (RV32I)
   ↓
Machine Code
   ↓
Instruction / Data Memory
   ↓
5-Stage RV32I CPU
   ↓
Snake Application
```

## Current Results

| Metric | Result |
|---|---:|
| Snake moves | 700 |
| Score | 56 |
| Length | 59 |
| Instructions | 11,555,161 |
| Cycles | 17,333,139 |
| CPI | 1.500 |
| Load-use stalls | 1,281,590 |
| Taken branches/jumps | 2,248,193 |

The Snake workload provides a repeatable application-level benchmark for studying **pipeline stalls, control hazards, and overall CPU performance**.

## Verification

The processor has also been tested using a C regression suite covering:

- Arithmetic and logical instructions
- Shifts
- Comparisons
- Loads and stores
- Branches
- Function pointers
- Memory initialization
- Sorting
- GCD
- Fibonacci
- Collatz
- Pointer chasing
- Data and read-only memory tests

**All regression tests passed.**

## Tools

- SystemVerilog
- Verilator
- RISC-V GCC Toolchain
- Make
- Vivado
- Xilinx Artix-7 FPGA

