// Copyright 1991-2014 Mentor Graphics Corporation

// All Rights Reserved.

// THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION
// WHICH IS THE PROPERTY OF MENTOR GRAPHICS CORPORATION
// OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.

// Testbench For the Ringbuf Design.

`timescale 1ns / 1ps

module test_ringbuf ();

    // Define Clock and Reset Signals
    reg clock;
    reg reset;
    wire txda, rxda, txc;

    initial begin
        reset <= 1'b0;
        #400 reset <= 1'b1;
    end

    // Clock Period set to 5MHz
    parameter clock_period = 200;
    initial
    begin
        clock <= 1'b0;
    end

    always
    begin : Clock_Generator
        #(clock_period / 2) clock = ~clock;
    end

    //
    // This section generates a pseudo-random data stream
    // that is used as the input to the ring buffer
    //
    reg [19:0] pseudo;

    // Use a 20 bit LFSR to generate data
    always @(posedge txc or negedge reset)
    begin : Generate_Data
        if (reset == 1'b0) begin
            pseudo <= 20'b00000000000000000000;
        end else begin
            pseudo <= {pseudo[18:0],pseudo[2] ^~ pseudo[19]};
        end
    end

    //
    // Assigns the output from the LFSR to the transmit data input
    // into the device
    //
    assign txda = pseudo[19];

    //
    // This section puts the data that comes out of the ring buf into
    // a LFSR with the same taps as the generated data. It then compares
    // this data with the next bit to ensure that the data is correct.
    //
    reg [19:0] storage;
    reg expected, dataerror;
    wire outstrobe;
    wire actual;

    assign actual = storage[0];

    always @ (negedge clock)
    begin : Compare_Data
        dataerror = actual ^~ expected;
        if (reset == 1'b0) begin
            storage <= 20'b00000000000000000000;
            expected <= 1'b0;
        end else begin
            if (outstrobe == 1'b1) begin
                storage <= {storage[18:0],rxda};
                expected <= storage[2] ^~ storage[19];
            end
        end
    end

    always @ (negedge dataerror)
    begin : print_error
        if ($time > 600)
            $display("** NOTICE ** at %1.0f ns: Data value not expected.\n", $realtime);
    end

    always @ (posedge dataerror)
    begin : print_restore
        if ($time > 600)
            $display("** RESTORED ** at %1.0f ns: Data returned to expected value.\n", $realtime);
    end

    // Instantiate the device
    ringbuf ring_INST (
               .clock (clock),
               .reset (reset),
               .txda (txda),
               .rxda (rxda),
               .txc (txc),
               .outstrobe(outstrobe) );

endmodule
