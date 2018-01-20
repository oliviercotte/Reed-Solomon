///////////////////////////////////////////////////////////////////////////////
///\\file     RSEncoderTB.h
///\\brief    
///
///           INF8500 - Laboratoire 2
///			  Automne 2009
///			  par Laurent Moss (moss_AT_grm.polymtl.ca)
///
///			  Modification Automne 2013
///			  par Etienne Gauthier (etienne-2.gauthier@polymtl.ca)
///
///           (c) All Rights Reserved. Ecole Polytechnique Montreal 2008,2013          
///                                                                                                                   
///
///////////////////////////////////////////////////////////////////////////////
#ifndef RSENCODERTB_H_
#define RSENCODERTB_H_

#include <systemc.h>
#include "RSCommon.h"


class RSEncoderTB : public sc_module{

public:

	//////////////////////////////////////
    // Ports standards
    //////////////////////////////////////
	sc_in_clk	clockPort;
	sc_out<bool> resetPort;

	//////////////////////////////////////
    // Ports de communication
    //////////////////////////////////////
	sc_out<sc_uint<8> >	dataOutputPort[NB_BYTE_INPUT];
	sc_in<bool>			encoderReadyPort;
	sc_in<bool>			encoderValidPort;
	sc_in<sc_uint<8> >	encoderInputPort[NB_BYTE_INPUT];

	//////////////////////////////////////
    // Constructeur / Destructeur
    //////////////////////////////////////
	RSEncoderTB(sc_module_name zName);
	~RSEncoderTB();

private:

	SC_HAS_PROCESS(RSEncoderTB);

	//////////////////////////////////////
    // Declaration des threads
    //////////////////////////////////////
	void thread();
	// Declarez d'autres thread ici au besoin
	
	//////////////////////////////////////
    // Declaration des methodes
    //////////////////////////////////////

    // Declarez vos methodes ici au besoin

	unsigned char galois(unsigned char in1, unsigned char in2);
	void generate_gf();
	void gen_poly();
	int  decode_rs();

    //////////////////////////////////////
    // Declaration des variables
    //////////////////////////////////////

    // Declarez vos variables ici au besoin	

	/* generator polynomial, tables for Galois field */
	int alpha_to[nn + 1], index_of[nn + 1], gg[nn - kk + 1];

	/* data[] is the info vector, bb[] is the parity vector, recd[] is the
	noise corrupted received vector  */
	int recd[nn], data[kk], bb[nn - kk];

};

#endif //RSENCODERTB_H_