///////////////////////////////////////////////////////////////////////////////
//
//	Main top level file
//
///////////////////////////////////////////////////////////////////////////////
#include <ctime>
#include <systemc.h>
#include "RSEncoder.h"
#include "RSEncoderTB.h"
#include <cstdlib>




///////////////////////////////////////////////////////////////////////////////
//
//	Entrée principale de l'application SystemC
//
///////////////////////////////////////////////////////////////////////////////
int sc_main(int arg_count, char **arg_value)
{
	srand(RANDOM_SEED);

	// Variable calcul de temps de simulation
	time_t simulation_time_begin = 0;
	time_t simulation_time_end = 0;
	int sim_units = 2; //SC_NS 

	// Horloge
	sc_clock clk("SysClock", 40, SC_NS, 0.5);

	// Reset signal
	sc_buffer<bool> resetPort;

	// Signaux de connexion entre le decodeur et le test bench
	sc_buffer<sc_uint<8>>	dataInputPort[16];
	sc_buffer<bool>			inputReadyPort;
	sc_buffer<bool>			outputValidPort;
	sc_buffer<sc_uint<8> >	encodingOutputPort[16];

	// Encodeur
	RSEncoder encoder("Encoder");

	// Test bench
	RSEncoderTB tb("EncoderTestBench");

	// Branchement encodeur
	encoder.clockPort(clk);
	encoder.resetPort(resetPort);
	for (unsigned int i = 0; i < 16; ++i){
		encoder.encoderInputPort[i](dataInputPort[i]);
		encoder.encoderOutputPort[i](encodingOutputPort[i]);
	}
	encoder.encoderReadyPort(inputReadyPort);
	encoder.encoderValidPort(outputValidPort);
	
	// Branchement test bench
	tb.clockPort(clk);
	tb.resetPort(resetPort);
	for (unsigned int i = 0; i < 16; ++i){
		tb.dataOutputPort[i](dataInputPort[i]);
		tb.encoderInputPort[i](encodingOutputPort[i]);
	}
	tb.encoderReadyPort(inputReadyPort);
	tb.encoderValidPort(outputValidPort);

	// Démarrage de l'application
	cout << "Demarrage de la simulation." << endl;
	time(&simulation_time_begin);
	sc_start(-1, sc_core::sc_time_unit(sim_units));
	time(&simulation_time_end);

	cout << endl << "Simulation s'est terminee a " << sc_time_stamp() << " ns";
	cout << endl << "Duree de la simulation: " << (unsigned long)(simulation_time_end - simulation_time_begin) << " secondes." << endl << endl;
	
	// Fin du programme
	return 0;
}