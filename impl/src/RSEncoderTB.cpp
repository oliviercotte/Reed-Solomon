///////////////////////////////////////////////////////////////////////////////
///\\file     RSEncoderTB.cpp
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
#include "RSEncoderTB.h"

//////////////////////////////////////////
// Initialisation des variables statiques
//////////////////////////////////////////

// Initialisez ici vos variables statiques au besoin

//////////////////////////////////////////
// Constructeur
//////////////////////////////////////////
RSEncoderTB::RSEncoderTB(sc_module_name zName)
	: sc_module(zName)
{
	SC_CTHREAD(thread, clockPort);

	// Initialisation: generation du champ de Galois
	printf("TB : Generation du champ de Galois GF(%d)\n\n", n1);
	generate_gf();
	gen_poly();
}

//////////////////////////////////////////
// Destructeur
//////////////////////////////////////////
RSEncoderTB::~RSEncoderTB()
{}

//////////////////////////////////////////
// Implementation des methodes
//////////////////////////////////////////

// Implementez ici vos methodes au besoin


//////////////////////////////////////////
// Implementation des threads
//////////////////////////////////////////
void RSEncoderTB::thread()
{
	std::cout << "Initialisation du TB" << endl;
	//Initialisation des sorties

	dataOutputPort[0].write(0);
	dataOutputPort[1].write(0);
	dataOutputPort[2].write(0);
	dataOutputPort[3].write(0);
	dataOutputPort[4].write(0);
	dataOutputPort[5].write(0);
	dataOutputPort[6].write(0);
	dataOutputPort[7].write(0);
	dataOutputPort[8].write(0);
	dataOutputPort[9].write(0);
	dataOutputPort[10].write(0);
	dataOutputPort[11].write(0);
	dataOutputPort[12].write(0);
	dataOutputPort[13].write(0);
	dataOutputPort[14].write(0);
	dataOutputPort[15].write(0);

	std::cout << "TB main loop start" << std::endl;

	//Met le reset sur un cycle
	resetPort.write(true);
	wait();
	resetPort.write(false);
	wait();

	///////////////////////////////////////
   // Comme dans la specification
   // executable, le(s) thread(s) doi(ven)t:
   // 
   // 1) Generer des paquets aleatoires
   // 2) Les transferer a l'encodeur
   // 3) Recuperer les paquets de sortie
   // 4) Y injecter des erreurs
   // 5) Decoder les paquets avec erreurs
   // 6) Verifier que les donnees peuvent
   //    etre recuperees.
   //
   //
   ///////////////////////////////////////
	int no_encoder_errors = 0;

	const unsigned int NUMBER_OF_TEST_VECTOR = 100;

	//#pragma omp parallel for \o/
	for (int errorNumberIT = 1; errorNumberIT <= CORRECTION_CAPACITY +1; ++errorNumberIT)
	{
		no_encoder_errors = 0;
		std::cout << "Envoie de " << NUMBER_OF_TEST_VECTOR << " vecteurs contenant " << errorNumberIT << " erreurs" << std::endl;
		for (int j = 0; j < NUMBER_OF_TEST_VECTOR; ++j)
		{

			///////////////////////////////////////
			// Comme dans la specification
			// executable, le(s) thread(s) doi(ven)t:
			// 
			// 1) Generer des paquets aleatoires

			// Creation d'un vecteur de test aleatoire (
			for (int i = 0; i < MESSAGE_LENGTH - 2; ++i)
			{
				data[i] = (int)(rand() % 256);
			}

			// 2) Les transferer a l'encodeur
			// Envoie des données du paquet (16 octets par cycle d'horloge)

			//Attente du "ready"
			while (!encoderReadyPort.read())
			{
				wait();
			}

			int counterIndex = 0;
			while (counterIndex < MESSAGE_LENGTH)
			{
				for (int i = 0; i < NB_BYTE_INPUT; ++i)
				{
					dataOutputPort[i].write(data[counterIndex]);
					counterIndex++;
				}
				wait();
			}

			//"Notez bien qu'il y a un cycle d'horloge entre le moment ou un module 
			//ecrit une valeur sur son port de sortie et le moment ou l'autre module voit cette valeur apparaitre
			//sur son port d'entree" --Gandhi
			while (!encoderValidPort.read())
			{
				wait();
			}

			// 3) Recuperer les paquets de sortie
			counterIndex = 0;
			while (counterIndex < BLOCK_LENGTH)
			{
				for (int i = 0; i < NB_BYTE_INPUT; ++i)
				{
					recd[counterIndex] = encoderInputPort[i].read();
					counterIndex++;
				}

				//On ne veut pas d'attente sur le dernier paquet
				wait();
			}

			// 4) Y injecter des erreurs
			
			int error_status[BLOCK_LENGTH];
			for (int i = 0; i < BLOCK_LENGTH; ++i)
			{
				error_status[i] = 0;
			}

			//Selection aleatoires de i erreurs : 
			for (int k = 0; k < errorNumberIT; ++k)
			{
				int error_index = (int)(rand() % 256);
				if (error_status[error_index] == 0) {
					// Injection d'une erreur a l'indice choisi
					recd[error_index] = (recd[error_index] ^ rand()) % 256;
					error_status[error_index] = 1;
				}
				else {
					k--;
				}
			}


			// 5) Decoder les paquets avec erreurs
			decode_rs();


			// 6) Verifier que les donnees peuvent
			//    etre recuperees.

			// Comparaison des donnees decodees avec les donnees
			// avec les donnes envoyees
			
			for (int k = 0; k < MESSAGE_LENGTH; k++) {
				if (data[k] != recd[k]) {
#ifdef DEBUG
					printf("\nErreur: valeur initiale etait de:\n");
					for (int i = 0; i < MESSAGE_LENGTH; ++i) {
						printf("%#x ", message[i].to_int());
					}
					printf("\n");
					printf("\nValeur recue est de:\n");
					for (int i = 0; i < MESSAGE_LENGTH; ++i) {
						printf("%#x ", rsEncoded[i].to_int());
					}
					printf("\nLes octets de party sont :\n");
					for (int i = 0; i < CORRECTION_CAPACITY * 2; ++i)
					{
						printf("%#x ", rsEncoded[i + MESSAGE_LENGTH].to_int());
					}
					printf("\n");

#endif
					// Les donnees ont ete mal encodees
					no_encoder_errors++;
					break;
				}
			}
			

			//
			//
			///////////////////////////////////////

		}

		if (no_encoder_errors == 0) {
			printf("OK\n");
		}
		else {
			printf("%d erreurs sur %d vecteurs.\n", no_encoder_errors, NUMBER_OF_TEST_VECTOR);
		}
	}
	sc_stop();
}

unsigned char RSEncoderTB::galois(unsigned char in1, unsigned char in2)
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

void RSEncoderTB::generate_gf()
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


void RSEncoderTB::gen_poly()
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

int RSEncoderTB::decode_rs()
{
	register int i, j, u, q;
	int elp[nn - kk + 2][nn - kk], d[nn - kk + 2], l[nn - kk + 2], u_lu[nn - kk + 2], s[nn - kk + 1];
	int count = 0, syn_error = 0, root[tt], loc[tt], z[tt + 1], err[nn], reg[tt + 1];

	/* recd[] is in polynomial form, convert to index form */
	for (i = 0; i < nn; i++) recd[i] = index_of[recd[i]];

	/* first form the syndromes; i.e., evaluate recd(x) at roots of g(x) namely
	@**(b0+i), i = 0, ... ,(2*tt-1) */
	for (i = 1; i <= nn - kk; i++)
	{
		s[i] = 0;
		for (j = 0; j < nn; j++)
		if (recd[j] != -1)
			s[i] ^= alpha_to[(recd[j] + (b0 + i - 1)*j) % nn];      /* recd[j] in index form */
		/* convert syndrome from polynomial form to index form  */
		if (s[i] != 0)  syn_error = 1;   /* set flag if non-zero syndrome => error */
		s[i] = index_of[s[i]];
	};

	if (syn_error)       /* if errors, try and correct */
	{
		/* compute the error location polynomial via the Berlekamp iterative algorithm,
		following the terminology of Lin and Costello :   d[u] is the 'mu'th
		discrepancy, where u = 'mu'+ 1 and 'mu' (the Greek letter!) is the step number
		ranging from -1 to 2*tt (see L&C),  l[u] is the
		degree of the elp at that step, and u_l[u] is the difference between the
		step number and the degree of the elp.

		The notation is the same as that in Lin and Costello's book; pages 155-156 and 175.

		*/
		/* initialise table entries */
		d[0] = 0;           /* index form */
		d[1] = s[1];        /* index form */
		elp[0][0] = 0;      /* index form */
		elp[1][0] = 1;      /* polynomial form */
		for (i = 1; i<nn - kk; i++)
		{
			elp[0][i] = -1;   /* index form */
			elp[1][i] = 0;   /* polynomial form */
		}
		l[0] = 0;
		l[1] = 0;
		u_lu[0] = -1;
		u_lu[1] = 0;
		u = 0;

		do
		{
			u++;
			if (d[u] == -1)
			{
				l[u + 1] = l[u];
				for (i = 0; i <= l[u]; i++)
				{
					elp[u + 1][i] = elp[u][i];
					elp[u][i] = index_of[elp[u][i]];
				}
			}
			else
				/* search for words with greatest u_lu[q] for which d[q]!=0 */
			{
				q = u - 1;
				while ((d[q] == -1) && (q>0)) q--;
				/* have found first non-zero d[q]  */
				if (q>0)
				{
					j = q;
					do
					{
						j--;
						if ((d[j] != -1) && (u_lu[q]<u_lu[j]))
							q = j;
					} while (j>0);
				};

				/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
				/* store degree of new elp polynomial */
				if (l[u]>l[q] + u - q)  l[u + 1] = l[u];
				else  l[u + 1] = l[q] + u - q;

				/* form new elp(x) */
				for (i = 0; i<nn - kk; i++)    elp[u + 1][i] = 0;
				for (i = 0; i <= l[q]; i++)
				if (elp[q][i] != -1)
					elp[u + 1][i + u - q] = alpha_to[(d[u] + nn - d[q] + elp[q][i]) % nn];
				for (i = 0; i <= l[u]; i++)
				{
					elp[u + 1][i] ^= elp[u][i];
					elp[u][i] = index_of[elp[u][i]];  /*convert old elp value to index*/
				}
			}
			u_lu[u + 1] = u - l[u + 1];

			/* form (u+1)th discrepancy */
			if (u<nn - kk)    /* no discrepancy computed on last iteration */
			{
				if (s[u + 1] != -1)
					d[u + 1] = alpha_to[s[u + 1]];
				else
					d[u + 1] = 0;
				for (i = 1; i <= l[u + 1]; i++)
				if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0))
					d[u + 1] ^= alpha_to[(s[u + 1 - i] + index_of[elp[u + 1][i]]) % nn];
				d[u + 1] = index_of[d[u + 1]];    /* put d[u+1] into index form */
			}
		} while ((u < nn - kk) && (l[u + 1] <= tt));


		u++;
		if (l[u] <= tt)         /* can correct error */
		{
			/* put elp into index form */
			for (i = 0; i <= l[u]; i++)   elp[u][i] = index_of[elp[u][i]];

			/* find roots of the error location polynomial */
			for (i = 1; i <= l[u]; i++)
				reg[i] = elp[u][i];
			count = 0;
			for (i = 1; i <= nn; i++)
			{
				q = 1;
				for (j = 1; j <= l[u]; j++)
				if (reg[j] != -1)
				{
					reg[j] = (reg[j] + j) % nn;
					q ^= alpha_to[reg[j]];
				};
				if (!q)        /* store root and error location number indices */
				{
					root[count] = i;
					loc[count] = nn - i;
					count++;
				};
			};
			if (count == l[u])    /* no. roots = degree of elp hence <= tt errors */
			{
				/* form polynomial z(x) */
				for (i = 1; i <= l[u]; i++)        /* Z[0] = 1 always - do not need */
				{
					if ((s[i] != -1) && (elp[u][i] != -1))
						z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]];
					else if ((s[i] != -1) && (elp[u][i] == -1))
						z[i] = alpha_to[s[i]];
					else if ((s[i] == -1) && (elp[u][i] != -1))
						z[i] = alpha_to[elp[u][i]];
					else
						z[i] = 0;
					for (j = 1; j<i; j++)
					if ((s[j] != -1) && (elp[u][i - j] != -1))
						z[i] ^= alpha_to[(elp[u][i - j] + s[j]) % nn];
					z[i] = index_of[z[i]];         /* put into index form */
				};

				/* evaluate errors at locations given by error location numbers loc[i] */
				for (i = 0; i<nn; i++)
				{
					err[i] = 0;
					if (recd[i] != -1)        /* convert recd[] to polynomial form */
						recd[i] = alpha_to[recd[i]];
					else  recd[i] = 0;
				}
				for (i = 0; i < l[u]; i++)    /* compute numerator of error term first */
				{
					err[loc[i]] = 1;       /* accounts for z[0] */
					for (j = 1; j <= l[u]; j++){
						if (z[j] != -1)
							err[loc[i]] ^= alpha_to[(z[j] + j*root[i]) % nn];
					} /* z(x) evaluated at X(l)**(-1) */
					if (err[loc[i]] != 0) /* term X(l)**(1-b0) */
						err[loc[i]] = alpha_to[(index_of[err[loc[i]]] + root[i] * (b0 + nn - 1)) % nn];
					if (err[loc[i]] != 0)
					{
						err[loc[i]] = index_of[err[loc[i]]];
						q = 0;     /* form denominator of error term */
						for (j = 0; j<l[u]; j++)
						if (j != i)
							q += index_of[1 ^ alpha_to[(loc[j] + root[i]) % nn]];
						q = q % nn;
						err[loc[i]] = alpha_to[(err[loc[i]] - q + nn) % nn];
						recd[loc[i]] ^= err[loc[i]];  /*recd[i] must be in polynomial form */
					}
				}
				return(1);
			}
			else{    /* no. roots != degree of elp => >tt errors and cannot solve */
				for (i = 0; i<nn; i++){        /* could return error flag if desired */
					if (recd[i] != -1)        /* convert recd[] to polynomial form */
						recd[i] = alpha_to[recd[i]];
					else
						recd[i] = 0;     /* just output received word as is */
				}
				return(2);
			}
		}
		else{         /* elp has degree has degree >tt hence cannot solve */
			for (i = 0; i<nn; i++){       /* could return error flag if desired */
				if (recd[i] != -1)        /* convert recd[] to polynomial form */
					recd[i] = alpha_to[recd[i]];
				else
					recd[i] = 0;     /* just output received word as is */
			}
			return(3);
		}
	}
	else{       /* no non-zero syndromes => no errors: output received codeword */
		for (i = 0; i < nn; i++){
			if (recd[i] != -1)        /* convert recd[] to polynomial form */
				recd[i] = alpha_to[recd[i]];
			else
				recd[i] = 0;
		}
		return(0);
	}
}