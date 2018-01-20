///////////////////////////////////////////////////////////////////////////////
///\\file     RSEncoder.cpp
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
#include "RSEncoder.h"

// Questa Sim export
#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(RSEncoder)
#endif

//////////////////////////////////////////
// Initialisation des variables statiques
//////////////////////////////////////////

// Initialisez ici vos variables statiques au besoin

RSEncoder::RSEncoder(sc_module_name zName )
	: sc_module(zName)
{
	SC_CTHREAD(thread, clockPort );
	// Configuration du reset pour la synthese
    reset_signal_is(resetPort, true);
	
	// Creez d'autres threads ici au besoin
	printf("ENCODER : Generation du champ de Galois GF(%d)\n\n", n1);
	generate_gf();
	gen_poly();
}

RSEncoder::~RSEncoder()
{}


//////////////////////////////////////////
// Implementation des methodes
//////////////////////////////////////////

// Implementez ici vos methodes au besoin


//////////////////////////////////////////
// Implementation des threads
//////////////////////////////////////////
void RSEncoder::thread()
{
	////////////////////////////////////////
	// Procedure d'initialisation
	////////////////////////////////////////

	encoderReadyPort.write(false);
	encoderValidPort.write(false);
  
	encoderOutputPort[0].write(0);
	encoderOutputPort[1].write(0);
	encoderOutputPort[2].write(0);
	encoderOutputPort[3].write(0);
	encoderOutputPort[4].write(0);
	encoderOutputPort[5].write(0);
	encoderOutputPort[6].write(0);
	encoderOutputPort[7].write(0);
	encoderOutputPort[8].write(0);
	encoderOutputPort[9].write(0);
	encoderOutputPort[10].write(0);
	encoderOutputPort[11].write(0);
	encoderOutputPort[12].write(0);
	encoderOutputPort[13].write(0);
	encoderOutputPort[14].write(0);
	encoderOutputPort[15].write(0);

	// Terminez l'initialisation de l'encodeur ici
	wait();
	
	while(1) 
	{

		/////////////////////////////////////
		// Protocole de lecture
		/////////////////////////////////////

		// Le banc d'essai doit commencer à envoyer à l'encodeur le contenu du paquet
		encoderReadyPort.write(true);
		wait();

		// Reception de 239 octets en bloc de 16 octets.
		int counterIndex = 0;
		while (counterIndex < MESSAGE_LENGTH)
		{
			wait();
			for (int i = 0; i < NB_BYTE_INPUT; ++i)
			{
				data[counterIndex] = encoderInputPort[i].read();
				counterIndex++;
			}
		}

		//Signal ready supposement recu du TB. 
		//Donne prete au prochain cycle!
		encoderReadyPort.write(false);

		/////////////////////////////////////
		// Encodage Reed-Solomon
		/////////////////////////////////////
		encode_rs();

		// Recuperation des octets redondants produits par l'encodeur
		for (int k = 0; k < nn - kk; k++) {
			recd[kk + k] = bb[k];
		}

		// Recuperation des octets envoyes
		for (int k = 0; k < kk; k++) {
			recd[k] = data[k];
		}

		/////////////////////////////////////
		// Protocole d'ecriture
		/////////////////////////////////////

		// L'encodeur est prêt à envoyer un nouveau paquet sur sa sortie
		encoderValidPort.write(true);

		// Envoie des données du paquet plus les octets redondants (16 octets par cycle d'horloge)
		counterIndex = 0;
		while (counterIndex < BLOCK_LENGTH)
		{
			for (int i = 0; i < NB_BYTE_INPUT; ++i)
			{
				encoderOutputPort[i].write(recd[counterIndex]);
				counterIndex++;
			}
			wait();
		}

		// L'encodeur à terminé d'écrire le paquet de sortie
		encoderValidPort.write(false);
	}
}


unsigned char RSEncoder::galois(unsigned char in1, unsigned char in2)
{
	unsigned char A0, A1, A2, A3, A4, A5, A6, A7, B0, B1, B2, B3, B4, B5, B6, B7, Z0, Z1, Z2, Z3, Z4, Z5, Z6, Z7;
	unsigned char Z;

	A0 = in1 & 0x1;
	B0 = in2 & 0x1;
	A1 = (in1 >> 1) & 0x1;
	B1 = (in2 >> 1) & 0x1;
	A2 = (in1 >> 2) & 0x1;
	B2 = (in2 >> 2) & 0x1;
	A3 = (in1 >> 3) & 0x1;
	B3 = (in2 >> 3) & 0x1;
	A4 = (in1 >> 4) & 0x1;
	B4 = (in2 >> 4) & 0x1;
	A5 = (in1 >> 5) & 0x1;
	B5 = (in2 >> 5) & 0x1;
	A6 = (in1 >> 6) & 0x1;
	B6 = (in2 >> 6) & 0x1;
	A7 = (in1 >> 7) & 0x1;
	B7 = (in2 >> 7) & 0x1;

	Z0 = (B0&A0) ^ (B1&A7) ^ (B2&A6) ^ (B3&A5) ^ (B4&A4) ^ (B5&A3) ^ (B5&A7) ^ (B6&A2) ^ (B6&A6) ^ (B6&A7) ^ (B7&A1) ^ (B7&A5) ^ (B7&A6) ^ (B7&A7);

	Z1 = (B0&A1) ^ (B1&A0) ^ (B2&A7) ^ (B3&A6) ^ (B4&A5) ^ (B5&A4) ^ (B6&A3) ^ (B6&A7) ^ (B7&A2) ^ (B7&A6) ^ (B7&A7);

	Z2 = (B0&A2) ^ (B1&A1) ^ (B1&A7) ^ (B2&A0) ^ (B2&A6) ^ (B3&A5) ^ (B3&A7) ^ (B4&A4) ^ (B4&A6) ^ (B5&A3) ^ (B5&A5) ^ (B5&A7) ^ (B6&A2) ^ (B6&A4) ^ (B6&A6) ^ (B6&A7) ^ (B7&A1) ^ (B7&A3) ^ (B7&A5) ^ (B7&A6);

	Z3 = (B0&A3) ^ (B1&A2) ^ (B1&A7) ^ (B2&A1) ^ (B2&A6) ^ (B2&A7) ^ (B3&A0) ^ (B3&A5) ^ (B3&A6) ^ (B4&A4) ^ (B4&A5) ^ (B4&A7) ^ (B5&A3) ^ (B5&A4) ^ (B5&A6) ^ (B5&A7) ^ (B6&A2) ^ (B6&A3) ^ (B6&A5) ^ (B6&A6) ^ (B7&A1) ^ (B7&A2) ^ (B7&A4) ^ (B7&A5);

	Z4 = (B0&A4) ^ (B1&A3) ^ (B1&A7) ^ (B2&A2) ^ (B2&A6) ^ (B2&A7) ^ (B3&A1) ^ (B3&A5) ^ (B3&A6) ^ (B3&A7) ^ (B4&A0) ^ (B4&A4) ^ (B4&A5) ^ (B4&A6) ^ (B5&A3) ^ (B5&A4) ^ (B5&A5) ^ (B6&A2) ^ (B6&A3) ^ (B6&A4) ^ (B7&A1) ^ (B7&A2) ^ (B7&A3) ^ (B7&A7);

	Z5 = (B0&A5) ^ (B1&A4) ^ (B2&A3) ^ (B2&A7) ^ (B3&A2) ^ (B3&A6) ^ (B3&A7) ^ (B4&A1) ^ (B4&A5) ^ (B4&A6) ^ (B4&A7) ^ (B5&A0) ^ (B5&A4) ^ (B5&A5) ^ (B5&A6) ^ (B6&A3) ^ (B6&A4) ^ (B6&A5) ^ (B7&A2) ^ (B7&A3) ^ (B7&A4);

	Z6 = (B0&A6) ^ (B1&A5) ^ (B2&A4) ^ (B3&A3) ^ (B3&A7) ^ (B4&A2) ^ (B4&A6) ^ (B4&A7) ^ (B5&A1) ^ (B5&A5) ^ (B5&A6) ^ (B5&A7) ^ (B6&A0) ^ (B6&A4) ^ (B6&A5) ^ (B6&A6) ^ (B7&A3) ^ (B7&A4) ^ (B7&A5);

	Z7 = (B0&A7) ^ (B1&A6) ^ (B2&A5) ^ (B3&A4) ^ (B4&A3) ^ (B4&A7) ^ (B5&A2) ^ (B5&A6) ^ (B5&A7) ^ (B6&A1) ^ (B6&A5) ^ (B6&A6) ^ (B6&A7) ^ (B7&A0) ^ (B7&A4) ^ (B7&A5) ^ (B7&A6);

	Z = Z0;
	Z |= (Z1 << 1);
	Z |= (Z2 << 2);
	Z |= (Z3 << 3);
	Z |= (Z4 << 4);
	Z |= (Z5 << 5);
	Z |= (Z6 << 6);
	Z |= (Z7 << 7);

	return Z;
}

void RSEncoder::generate_gf()
{
	register int i, mask;

	mask = 1;
	alpha_to[mm] = 0;
	for (i = 0; i<mm; i++)
	{
		alpha_to[i] = mask;
		index_of[alpha_to[i]] = i;
		if (pp[i] != 0) /* If pp[i] == 1 then, term @^i occurs in poly-repr of @^mm */
			alpha_to[mm] ^= mask;  /* Bit-wise EXOR operation */
		mask <<= 1; /* single left-shift */
	}
	index_of[alpha_to[mm]] = mm;
	/* Have obtained poly-repr of @^mm. Poly-repr of @^(i+1) is given by
	poly-repr of @^i shifted left one-bit and accounting for any @^mm
	term that may occur when poly-repr of @^i is shifted. */
	mask >>= 1;
	for (i = mm + 1; i<nn; i++)
	{
		if (alpha_to[i - 1] >= mask)
			alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i - 1] ^ mask) << 1);
		else alpha_to[i] = alpha_to[i - 1] << 1;
		index_of[alpha_to[i]] = i;
	}
	index_of[0] = -1;
}


void RSEncoder::gen_poly()
/* Obtain the generator polynomial of the tt-error correcting, length
nn=(2**mm -1) Reed Solomon code from the product of (X+@**(b0+i)), i = 0, ... ,(2*tt-1)
Examples: 	If b0 = 1, tt = 1. deg(g(x)) = 2*tt = 2.
g(x) = (x+@) (x+@**2)
If b0 = 0, tt = 2. deg(g(x)) = 2*tt = 4.
g(x) = (x+1) (x+@) (x+@**2) (x+@**3)
*/
{
	register int i, j;

	gg[0] = alpha_to[b0];
	gg[1] = 1;    /* g(x) = (X+@**b0) initially */
	for (i = 2; i <= nn - kk; i++)
	{
		gg[i] = 1;
		/* Below multiply (gg[0]+gg[1]*x + ... +gg[i]x^i) by (@**(b0+i-1) + x) */
		for (j = i - 1; j>0; j--)
		if (gg[j] != 0)  gg[j] = gg[j - 1] ^ alpha_to[((index_of[gg[j]]) + b0 + i - 1) % nn];
		else gg[j] = gg[j - 1];
		gg[0] = alpha_to[((index_of[gg[0]]) + b0 + i - 1) % nn];     /* gg[0] can never be zero */
	}
	/* convert gg[] to index form for quicker encoding */
	for (i = 0; i <= nn - kk; i++)  gg[i] = index_of[gg[i]];
}

void RSEncoder::encode_rs()
/* take the string of symbols in data[i], i=0..(k-1) and encode systematically
to produce 2*tt parity symbols in bb[0]..bb[2*tt-1]
data[] is input and bb[] is output in polynomial form.
Encoding is done by using a feedback shift register with appropriate
connections specified by the elements of gg[], which was generated above.
Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)          */
{
	register int i, j;
	int feedback;

	for (i = 0; i<nn - kk; i++)   bb[i] = 0;
	for (i = kk - 1; i >= 0; i--)
	{
		feedback = data[i] ^ bb[nn - kk - 1];
		if (feedback != 0) /* feedback term is non-zero */
		{
			for (j = nn - kk - 1; j>0; j--)
				bb[j] = bb[j - 1] ^ galois(alpha_to[gg[j]], feedback);
			bb[0] = galois(alpha_to[gg[0]], feedback);
		}
		else /* feedback term is zero. encoder becomes a single-byte shifter */
		{
			for (j = nn - kk - 1; j>0; j--)
				bb[j] = bb[j - 1];
			bb[0] = 0;
		};
	};
}