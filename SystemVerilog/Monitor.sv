
module Monitor (test_RSEncoder tb);
//----------------------------------------------------------------
// Assertions
//-----------

// 1. Le signal encoderValidPort s’active bien 16 cycles après que le signal encoderReadyPort s’est activé. 
property p_ready_then_valid;
  @(posedge tb.clock) disable iff (tb.reset)
	$rose(tb.encoderReadyPort) ##16 $rose(tb.encoderValidPort) |=> !tb.encoderValidPort && !tb.encoderReadyPort;
endproperty
a_ready_then_valid: assert property (p_ready_then_valid);

// 2. Le signal encoderReadyPort s’active bien 16 cycles après que le signal encoderValidPort s’est activé.
property p_valid_then_ready;
  @(posedge tb.clock) disable iff (tb.reset)
    $rose(tb.encoderValidPort) ##16 $rose(tb.encoderReadyPort) |=> !tb.encoderValidPort && !tb.encoderReadyPort;
endproperty
a_valid_then_ready: assert property (p_valid_then_ready);

// 3. Le signal encoderReadyPort se désactive bien après avoir terminé de recevoir le paquet d’entrée.
property p_end_receive_packets;
  @(posedge tb.clock) disable iff (tb.reset)
	$rose(tb.encoderReadyPort) |-> ##15 $fell(tb.encoderReadyPort) and stable(tb.encoderInputPort);
endproperty
a_end_receive_packets: assert property (p_end_receive_packets);

// 4. Le signal encoderValidPort se désactive bien après avoir terminé d’écrire le paquet de sortie.
property p_end_send_packets;
  @(posedge tb.clock) disable iff (tb.reset)
	$rose(tb.encoderValidPort) |-> ##16 $fell(tb.encoderValidPort) and stable(tb.dataOutputPort);
endproperty
a_end_send_packets: assert property (p_end_send_packets);

// 5. Les signaux encoderReadyPort et encoderValidPort sont mutuellement exclusifs. 
property p_mutex_valid_ready;
  @(tb.clock) disable iff (tb.reset)
	!(!tb.encoderValidPort && !tb.encoderReadyPort) && !(tb.encoderValidPort && tb.encoderReadyPort);
endproperty
a_mutex_valid_ready: assert property (p_mutex_valid_ready);

endmodule
