# RV32I Processor — FPGA Baseline

> **Baseline RTL used for Snake = exact current RTL from `C:\Users\sharu\Desktop\riscv\fpgaimpimentation`**

---

## Configuration

- **Project**: In-Memory Baseline Flow (`run_baseline.tcl`)
- **FPGA Part**: `xc7a100tcsg324-1` (Xilinx Artix-7, Digilent Nexys A7-100T)
- **Vivado Version**: Vivado v2025.1 (64-bit, Build 6140274)
- **Top Module**: `rv32i_core_pipelined_full`
- **Clock Port**: `clk` (Pin `E3`, 100 MHz oscillator pad)
- **Reset Port**: `rst_n` (Pin `C12`, active-low `CPU_RESETN`)
- **LED Ports**: `led_out[15:0]` (Pins `H17`, `K15`, `J13`, `N14`, `R18`, `V17`, `U17`, `U16`, `V16`, `T15`, `U14`, `T16`, `V15`, `V14`, `V12`, `V11`)
- **Active Constraint File**: `core.xdc`

### Verified RTL Source File List (19 Files)
All 19 source files were read strictly and exclusively from `C:\Users\sharu\Desktop\riscv\fpgaimpimentation`:
1. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/alu.sv`
2. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/branch_jump_unit.sv`
3. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/control_unit.sv`
4. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/data_mem_bram.sv`
5. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/ex_mem_reg.sv`
6. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/forwarding_unit.sv`
7. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/gpio_leds.sv`
8. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/hazard_detection_unit.sv`
9. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/id_ex_reg.sv`
10. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/if_id_reg.sv`
11. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/imm_gen.sv`
12. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/instr_decode_fields.sv`
13. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/instr_mem_bram.sv`
14. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/mem_wb_reg.sv`
15. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/operand_muxes.sv`
16. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/pc_next_gen.sv`
17. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/pc_reg.sv`
18. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/reg_file.sv`
19. `C:/Users/sharu/Desktop/riscv/fpgaimpimentation/rv32i_core_pipelined_full.sv`

---

## Timing

Post-route timing analysis was performed across all three target clock frequencies on the fully placed and routed design:

| Frequency Target | Clock Period | WNS | TNS | Failing Endpoints | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **100.000 MHz** | 10.000 ns | **-5.060 ns** | -721.513 ns | 182 / 1405 | **VIOLATED** |
| **68.966 MHz (core.xdc)** | 14.500 ns | **-0.560 ns** | -31.950 ns | 119 / 1405 | **VIOLATED** |
| **66.667 MHz** | 15.000 ns | **-0.060 ns** | -0.193 ns | 9 / 1405 | **VIOLATED** |

### Post-Route Detailed Timing (Under 14.5 ns Constraint)

| Metric | Result |
| :--- | ---: |
| Clock Period (Constrained) | 14.500 ns |
| Constrained Frequency | 68.966 MHz |
| **Maximum Achievable Frequency ($F_{\max}$)** | **66.401 MHz (Period: 15.060 ns)** |
| Worst Negative Slack (WNS) | -0.560 ns |
| Total Negative Slack (TNS) | -31.950 ns |
| Total Failing Endpoints | 119 / 1405 |
| Total Data Path Delay | 14.424 ns |
| Logic Delay | 3.942 ns (27.33%) |
| Routing Delay | 10.482 ns (72.67%) |
| Number of Logic Levels | 12 (LUT4=1, LUT5=1, LUT6=10) |
| Clock Path Skew | -0.035 ns |
| Clock Uncertainty | 0.035 ns |
| Library Setup Time Requirement | 0.566 ns |

---

## Utilization

| Resource | Used | Available | Utilization (%) |
| :--- | ---: | ---: | ---: |
| **Slice LUTs** | **1,370** | 63,400 | 2.16% |
| ↳ LUT as Logic | 1,326 | 63,400 | 2.09% |
| ↳ LUT as Distributed RAM (`reg_file`) | 44 | 19,000 | 0.23% |
| **Slice Registers (FF)** | **487** | 126,800 | 0.38% |
| ↳ Flip-Flops (`FDRE`) | 487 | 126,800 | 0.38% |
| ↳ Latches | 0 | 126,800 | 0.00% |
| **Block RAM Tile** | **8.0** | 135 | 5.93% |
| ↳ `RAMB36E1` (`u_dmem`: 4, `u_imem`: 4) | 8 | 135 | 5.93% |
| ↳ `RAMB18E1` | 0 | 270 | 0.00% |
| **DSPs** | **0** | 240 | 0.00% |
| **Bonded I/O Pins** | **18** | 210 | 8.57% |
| **Clock Buffers (BUFG)** | **1** | 32 | 3.13% |

### Hierarchical Resource Distribution
- `u_idex` (`id_ex_reg` + decoded logic): 778 LUTs, 190 FFs
- `u_memwb` (`mem_wb_reg`): 194 LUTs, 72 FFs
- `u_exmem` (`ex_mem_reg`): 157 LUTs, 108 FFs
- `u_ifid` (`if_id_reg`): 140 LUTs, 64 FFs
- `u_dmem` (`data_mem_bram`): 4 RAMB36E1
- `u_imem` (`instr_mem_bram`): 4 RAMB36E1

---

## Power

Power analysis was performed using Vivado vector-less activity propagation with default 12.5% toggle rates at 25.0 °C ambient:

| Metric | Result |
| :--- | ---: |
| **Total On-Chip Power** | **0.144 W (144 mW)** |
| **Dynamic Power** | **0.047 W (47 mW)** |
| **Device Static Power** | **0.098 W (98 mW)** |
| ↳ Signals | 0.020 W (20 mW) |
| ↳ Block RAM | 0.012 W (12 mW) |
| ↳ Slice Logic (LUTs) | 0.011 W (11 mW) |
| ↳ Clocks | 0.004 W (4 mW) |
| ↳ I/O | < 0.001 W (< 1 mW) |
| Junction Temperature | 25.7 °C |

---

## Critical Path Analysis

### Worst Path Summary
- **Startpoint**: `u_dmem/g_bank[2].mem_reg/CLKARDCLK` (`RAMB36E1`, data memory byte-lane 2)
- **Endpoint**: `regs_reg_r1_0_31_0_5_i_19/ADDRARDADDR[11]` (`RAMB36E1`, instruction memory fetch address)
- **Path Group**: `clk`
- **Path Type**: Setup (Max at Slow Process Corner)
- **Data Path Delay**: 14.424 ns
- **Logic Delay**: 3.942 ns (27.33%)
- **Routing Delay**: 10.482 ns (72.67%)

### Functional Progression
The critical timing path is dominated by:
$$\text{Data Memory Read (BRAM)} \rightarrow \text{Byte-Lane Alignment/Rotation} \rightarrow \text{WB Forwarding Muxes} \rightarrow \text{ALU/Comparator} \rightarrow \text{Next-PC Branch Target Logic} \rightarrow \text{Instruction Memory BRAM Address}$$

1. **BRAM Clock-to-Out**: The data memory BRAM output delay contributes `2.454 ns` (62.3% of total logic delay).
2. **Byte-Lane Alignment & Sign Extension**: The output of the BRAM bank traverses rotation and sign-extension multiplexers in `u_dmem` (`0.248 ns` logic, `1.802 ns` routing).
3. **Forwarding to EX**: The load data is forwarded to the EX stage operands via `u_memwb` / `u_idex` (`rd1_fwd`).
4. **ALU & Branch Resolution**: Operands enter the arithmetic/comparison units in `u_alu` / `u_bju`.
5. **PC Target & Next-PC Mux**: The branch/jump target is evaluated to form `pc_next`.
6. **Instruction Memory Address Port**: `pc_next` is routed across the chip to the address port `ADDRARDADDR[11]` of the instruction memory Block RAM `u_imem`.

### Dominant Contributors
- **Routing Delay**: Accounts for **72.67%** of the data path delay, caused by spanning from Block RAM columns at the bottom across slices to the opposite Block RAM column.
- **BRAM Clock-to-Out without Output Pipeline Register**: The BRAM primitive lacks an embedded registered output (`SYNTH-6`), leaving raw RAM array access directly connected to combinational forwarding and next-PC evaluation.

---

## Warnings / Issues

1. **SYNTH-6 (Medium Severity)**:
   - *Message*: `Timing of a RAM block might be sub-optimal: The timing for instance u_dmem/g_bank[0..3].mem_reg might be sub-optimal as no output register was merged into the block.`
   - *Impact*: Directly creates the 2.454 ns launch delay on the critical path.
2. **TIMING-18 (Low/Informational)**:
   - *Message*: Missing input delay on `rst_n` (1 port) and output delay on `led_out[15:0]` (16 ports).
   - *Impact*: These are asynchronous human-interface I/Os on the Nexys A7 board (pushbutton and LEDs) and do not impact internal core timing.
3. **CFGBVS-1 (DRC Warning)**:
   - *Message*: Missing `CFGBVS` and `CONFIG_VOLTAGE` properties in `core.xdc`. Standard configuration warning.
4. **Timing Violations at 100 MHz**:
   - The design **cannot run at the board's native 100 MHz clock** without pipeline modifications or clock division. WNS is -5.060 ns at 100 MHz.

---

## Baseline Conclusion

The current RV32I pipelined processor successfully synthesizes and implements on the Artix-7 `xc7a100tcsg324-1` device using only **2.16% of LUTs**, **0.38% of FFs**, and **8 BRAM tiles** with a total on-chip power of **144 mW**.

However, the architecture has an unpipelined loop between **Data Memory load $\rightarrow$ Forwarding $\rightarrow$ ALU / Branch target $\rightarrow$ Instruction Memory address**. Consequently:
- **Maximum Achievable Clock Frequency ($F_{\max}$)**: **66.401 MHz** (limiting period: **15.060 ns**).
- The processor **cannot meet 100 MHz** (WNS = -5.060 ns).
- The processor narrowly misses the existing 14.5 ns target (WNS = -0.560 ns) and 15.0 ns target (WNS = -0.060 ns).
- Safe error-free operation on the Nexys A7 board requires either operating at $\le 66.4\text{ MHz}$ (e.g. via an MMCM/PLL generating ~50 MHz or 66 MHz) or adding a pipeline register to break the load-to-PC-next timing path.
