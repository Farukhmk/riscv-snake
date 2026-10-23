`timescale 1ns/1ps
// Runs program.hex / data_b*.hex on the core. Console = store to 0xFFFFFFF4,
// exit = store to 0xFFFFFFF8. Reads internal signals hierarchically; no RTL edits.
module tb_c_program;
    localparam logic [31:0] CONSOLE = 32'hFFFF_FFF4;
    localparam logic [31:0] EXITREG = 32'hFFFF_FFF8;
    localparam logic [31:0] FRAMESYNC = 32'hFFFF_FFFC;   // C writes here after each frame
    localparam longint MAX_CYCLES = 200_000_000;
    int delay_ms = 0;                                    // run with +delay_ms=100 to slow down
    initial if (!$value$plusargs("delay_ms=%d", delay_ms)) delay_ms = 0;

    logic clk = 0, rst_n = 0;
    logic [15:0] led_out;
    rv32i_core_pipelined_full dut (.clk(clk), .rst_n(rst_n), .led_out(led_out));
    always #5 clk = ~clk;

    longint cycles = 0, retired = 0, lu_stalls = 0, taken = 0, loads = 0, stores = 0;
    logic v_id = 0, v_ex = 0, v_mem = 0, v_wb = 0;   // shadow valid bits

    initial begin repeat (5) @(posedge clk); rst_n = 1; end

    always @(posedge clk) if (rst_n) begin
        cycles++;
        if (dut.hazard_stall) lu_stalls++;
        if (dut.if_id_flush)  taken++;
        if (v_mem && dut.mem_we_mem) stores++;
        if (v_mem && dut.result_src_mem == 2'b01) loads++;
        if (v_wb) retired++;
        v_wb  <= v_mem;
        v_mem <= v_ex;
        v_ex  <= (dut.hazard_stall || dut.if_id_flush) ? 1'b0 : v_id;
        v_id  <= dut.hazard_stall ? v_id : (dut.if_id_flush ? 1'b0 : 1'b1);

        if (dut.mem_we_mem && dut.alu_result_mem == CONSOLE)
            $write("%c", dut.store_data_mem[7:0]);

        if (dut.mem_we_mem && dut.alu_result_mem == FRAMESYNC && delay_ms > 0) begin
            $fflush;                                     // show the frame now
            $system($sformatf("sleep %0d.%03d", delay_ms / 1000, delay_ms % 1000));
        end

        if (dut.mem_we_mem && dut.alu_result_mem == EXITREG) begin
            $display("\n===============================");
            $display("EXIT code    : %0d", dut.store_data_mem);
            $display("Instructions : %0d", retired);
            $display("Cycles       : %0d", cycles);
            $display("CPI          : %f", real'(cycles) / real'(retired));
            $display("Load-use stalls : %0d", lu_stalls);
            $display("Taken br/jumps  : %0d (flushes)", taken);
            $display("Loads / Stores  : %0d / %0d", loads, stores);
            $display("===============================");
            $finish;
        end
        if (cycles >= MAX_CYCLES) begin $display("TIMEOUT"); $finish; end
    end
endmodule