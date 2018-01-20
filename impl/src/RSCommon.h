///////////////////////////////////////////////////////////////////////////////
///\\file     RSCommon.h
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
#ifndef RSCOMMON_H_
#define RSCOMMON_H_

#include "systemc.h"

// Device configuration
const unsigned int	NB_BYTE_INPUT = 16;
const unsigned int  RANDOM_SEED = 123456;

// Reed-Solomon Algorythm configuration       
#define mm  8           /* RS code over GF(2**mm) - change to suit */
#define n1   256   		/* n = size of the field */
#define nn  255         /* nn=2**mm -1   length of codeword */
#define tt  8           /* number of errors that can be corrected */
#define kk  239         /* kk = nn-2*tt  */ /* Degree of g(x) = 2*tt */
#define b0  0           /* g(x) has roots @**b0, @**(b0+1), ... ,@^(b0+2*tt-1) */


const unsigned int  NB_SYMBOL_BITS = 8; // m /* RS code over GF(2**mm) - change to suit */ // GF(q) q = 2**m = 2**8 , n = q ou n = q-1
const unsigned int	BLOCK_LENGTH = nn; // n = 2**m - 1 = 2**8 - 1 = 255
const unsigned int	CORRECTION_CAPACITY = tt; // tt mm 8 symbole can be corrected
const unsigned int	MESSAGE_LENGTH = BLOCK_LENGTH - 2 * CORRECTION_CAPACITY; // (k) message length < block length (n)


/**** Primitive polynomial ****/
// Polynome premier (ireductible) servant de base pour generer les corps de galoies GF(p^m) de degre m;
static sc_uint<1>  pp[NB_SYMBOL_BITS + 1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1 }; /* 1+x^2+x^3+x^4+x^8 */


#endif // RSCOMMON_H_
