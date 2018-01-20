/*
 * File:     general_RS.c
 * Title:    Encoder/decoder for RS codes in C
 * Authors:  Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu)
 *           Hari Thirumoorthy (harit@spectra.eng.hawaii.edu)
 *           and Laurent Moss (laurent.moss@polymtl.ca) (adapted for
 *           a lab of course INF8500 at Ecole Polytechnique de Montreal
 * Date:     Aug 1995 (original), Sep 2009 (adaptation)
 *
 *
 * ===============  Encoder/Decoder for RS codes in C =================
 *
 *
 * The encoding and decoding methods used in this program are based on the
 * book "Error Control Coding: Fundamentals and Applications", by Lin and
 * Costello, Prentice Hall, 1983. Most recently on "Theory and Practice of 
 * Error Control Codes", by R.E. Blahut.
 *
 *
 * NOTE:    
 *          The authors are not responsible for any malfunctioning of
 *          this program, nor for any damage caused by it. Please include the
 *          original program along with these comments in any redistribution.
 *
 * Portions of this program are from a Reed-Solomon encoder/decoder
 * in C, written by Simon Rockliff (simon@augean.ua.oz.au) on 21/9/89.
 *
 * COPYRIGHT NOTICE: This computer program is free for non-commercial purposes.
 * You may implement this program for any non-commercial application. You may 
 * also implement this program for commercial purposes, provided that you
 * obtain my written permission. Any modification of this program is covered
 * by this copyright.
 *
 * Copyright (c) 1995.  Robert Morelos-Zaragoza and Hari Thirumoorthy.
 * Copyright (c) 2009.  Laurent Moss.
 *                      All rights reserved.
 *
 */

/*
  Program computes the generator polynomial of a RS code. Also performs 
encoding and decoding of the RS code.
 
  FUNCTIONS:
		generate_gf() generates the field.
		gen_poly() generates the generator polynomial.
		encode_rs() encodes in systematic form.
		decode_rs() errors-only decodes a vector assumed to be encoded in systematic form.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define mm  8           /* RS code over GF(2**mm) - change to suit */
#define n1   256   		/* n = size of the field */
#define nn  255         /* nn=2**mm -1   length of codeword */
#define tt  8           /* number of errors that can be corrected */
#define kk  239         /* kk = nn-2*tt  */ /* Degree of g(x) = 2*tt */
#define b0  0           /* g(x) has roots @**b0, @**(b0+1), ... ,@^(b0+2*tt-1) */

/**** Primitive polynomial ****/
int pp [mm+1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1}; /* 1+x^2+x^3+x^4+x^8 */

/* generator polynomial, tables for Galois field */
int alpha_to [nn+1], index_of [nn+1], gg [nn-kk+1];

/* data[] is the info vector, bb[] is the parity vector, recd[] is the 
  noise corrupted received vector  */
int recd[nn], data[kk], bb[nn-kk] ;

/* generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**m)
   HARI's COMMENT: (4/13/94) alpha_to[] can be used as follows:
        Let @ represent the primitive element commonly called "alpha" that
   is the root of the primitive polynomial p(x). Then in GF(2^m), for any
   0 <= i <= 2^m-2,
        @^i = a(0) + a(1) @ + a(2) @^2 + ... + a(m-1) @^(m-1)
   where the binary vector (a(0),a(1),a(2),...,a(m-1)) is the representation
   of the integer "alpha_to[i]" with a(0) being the LSB and a(m-1) the MSB. Thus for
   example the polynomial representation of @^5 would be given by the binary
   representation of the integer "alpha_to[5]".
                   Similarily, index_of[] can be used as follows:
        As above, let @ represent the primitive element of GF(2^m) that is
   the root of the primitive polynomial p(x). In order to find the power
   of @ (alpha) that has the polynomial representation
        a(0) + a(1) @ + a(2) @^2 + ... + a(m-1) @^(m-1)
   we consider the integer "i" whose binary representation with a(0) being LSB
   and a(m-1) MSB is (a(0),a(1),...,a(m-1)) and locate the entry
   "index_of[i]". Now, @^index_of[i] is that element whose polynomial 
    representation is (a(0),a(1),a(2),...,a(m-1)).
   NOTE:
        The element alpha_to[2^m-1] = 0 always signifying that the
   representation of "@^infinity" = 0 is (0,0,0,...,0).
        Similarily, the element index_of[0] = -1 always signifying
   that the power of alpha which has the polynomial representation
   (0,0,...,0) is "infinity".
 
*/

unsigned char galois(unsigned char in1, unsigned char in2)
{
	unsigned char A0,A1,A2,A3,A4,A5,A6,A7,B0,B1,B2,B3,B4,B5,B6,B7,Z0,Z1,Z2,Z3,Z4,Z5,Z6,Z7;
      	unsigned char Z;
	
	A0= in1 & 0x1;
	B0= in2 & 0x1;
	A1= (in1 >> 1) & 0x1;
	B1= (in2 >> 1) & 0x1;
	A2= (in1 >> 2) & 0x1;
	B2= (in2 >> 2) & 0x1;
	A3= (in1 >> 3) & 0x1;
	B3= (in2 >> 3) & 0x1;
	A4= (in1 >> 4) & 0x1;
	B4= (in2 >> 4) & 0x1;
	A5= (in1 >> 5) & 0x1;
	B5= (in2 >> 5) & 0x1;
	A6= (in1 >> 6) & 0x1;
	B6= (in2 >> 6) & 0x1;
	A7= (in1 >> 7) & 0x1;
	B7= (in2 >> 7) & 0x1;
	
	Z0=(B0&A0)^(B1&A7)^(B2&A6)^(B3&A5)^(B4&A4)^(B5&A3)^(B5&A7)^(B6&A2)^(B6&A6)^(B6&A7)^(B7&A1)^(B7&A5)^(B7&A6)^(B7&A7);
	
	Z1=(B0&A1)^(B1&A0)^(B2&A7)^(B3&A6)^(B4&A5)^(B5&A4)^(B6&A3)^(B6&A7)^(B7&A2)^(B7&A6)^(B7&A7);
	
	Z2=(B0&A2)^(B1&A1)^(B1&A7)^(B2&A0)^(B2&A6)^(B3&A5)^(B3&A7)^(B4&A4)^(B4&A6)^(B5&A3)^(B5&A5)^(B5&A7)^(B6&A2)^(B6&A4)^(B6&A6)^(B6&A7)^(B7&A1)^(B7&A3)^(B7&A5)^(B7&A6);
	
	Z3=(B0&A3)^(B1&A2)^(B1&A7)^(B2&A1)^(B2&A6)^(B2&A7)^(B3&A0)^(B3&A5)^(B3&A6)^(B4&A4)^(B4&A5)^(B4&A7)^(B5&A3)^(B5&A4)^(B5&A6)^(B5&A7)^(B6&A2)^(B6&A3)^(B6&A5)^(B6&A6)^(B7&A1)^(B7&A2)^(B7&A4)^(B7&A5);
	
	Z4=(B0&A4)^(B1&A3)^(B1&A7)^(B2&A2)^(B2&A6)^(B2&A7)^(B3&A1)^(B3&A5)^(B3&A6)^(B3&A7)^(B4&A0)^(B4&A4)^(B4&A5)^(B4&A6)^(B5&A3)^(B5&A4)^(B5&A5)^(B6&A2)^(B6&A3)^(B6&A4)^(B7&A1)^(B7&A2)^(B7&A3)^(B7&A7);
	
	Z5=(B0&A5)^(B1&A4)^(B2&A3)^(B2&A7)^(B3&A2)^(B3&A6)^(B3&A7)^(B4&A1)^(B4&A5)^(B4&A6)^(B4&A7)^(B5&A0)^(B5&A4)^(B5&A5)^(B5&A6)^(B6&A3)^(B6&A4)^(B6&A5)^(B7&A2)^(B7&A3)^(B7&A4);
	
	Z6=(B0&A6)^(B1&A5)^(B2&A4)^(B3&A3)^(B3&A7)^(B4&A2)^(B4&A6)^(B4&A7)^(B5&A1)^(B5&A5)^(B5&A6)^(B5&A7)^(B6&A0)^(B6&A4)^(B6&A5)^(B6&A6)^(B7&A3)^(B7&A4)^(B7&A5);
	
	Z7=(B0&A7)^(B1&A6)^(B2&A5)^(B3&A4)^(B4&A3)^(B4&A7)^(B5&A2)^(B5&A6)^(B5&A7)^(B6&A1)^(B6&A5)^(B6&A6)^(B6&A7)^(B7&A0)^(B7&A4)^(B7&A5)^(B7&A6);

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

void generate_gf()
 {
   register int i, mask ;

  mask = 1 ;
  alpha_to[mm] = 0 ;
  for (i=0; i<mm; i++)
   { alpha_to[i] = mask ;
     index_of[alpha_to[i]] = i ;
     if (pp[i]!=0) /* If pp[i] == 1 then, term @^i occurs in poly-repr of @^mm */
       alpha_to[mm] ^= mask ;  /* Bit-wise EXOR operation */
     mask <<= 1 ; /* single left-shift */
   }
  index_of[alpha_to[mm]] = mm ;
  /* Have obtained poly-repr of @^mm. Poly-repr of @^(i+1) is given by 
     poly-repr of @^i shifted left one-bit and accounting for any @^mm 
     term that may occur when poly-repr of @^i is shifted. */
  mask >>= 1 ;
  for (i=mm+1; i<nn; i++)
   { if (alpha_to[i-1] >= mask)
        alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i-1]^mask)<<1) ;
     else alpha_to[i] = alpha_to[i-1]<<1 ;
     index_of[alpha_to[i]] = i ;
   }
  index_of[0] = -1 ;
 }


void gen_poly()
/* Obtain the generator polynomial of the tt-error correcting, length
  nn=(2**mm -1) Reed Solomon code from the product of (X+@**(b0+i)), i = 0, ... ,(2*tt-1)
  Examples: 	If b0 = 1, tt = 1. deg(g(x)) = 2*tt = 2.
 	g(x) = (x+@) (x+@**2)
		If b0 = 0, tt = 2. deg(g(x)) = 2*tt = 4.
	g(x) = (x+1) (x+@) (x+@**2) (x+@**3)  	
*/
 {
   register int i,j ;

   gg[0] = alpha_to[b0];
   gg[1] = 1 ;    /* g(x) = (X+@**b0) initially */
   for (i=2; i <= nn-kk; i++)
    { gg[i] = 1 ;
      /* Below multiply (gg[0]+gg[1]*x + ... +gg[i]x^i) by (@**(b0+i-1) + x) */
      for (j=i-1; j>0; j--)
        if (gg[j] != 0)  gg[j] = gg[j-1]^ alpha_to[((index_of[gg[j]])+b0+i-1)%nn] ;
        else gg[j] = gg[j-1] ;
      gg[0] = alpha_to[((index_of[gg[0]])+b0+i-1)%nn] ;     /* gg[0] can never be zero */
    }
   /* convert gg[] to index form for quicker encoding */
   for (i=0; i <= nn-kk; i++)  gg[i] = index_of[gg[i]] ;
 }


void encode_rs()
/* take the string of symbols in data[i], i=0..(k-1) and encode systematically
   to produce 2*tt parity symbols in bb[0]..bb[2*tt-1]
   data[] is input and bb[] is output in polynomial form.
   Encoding is done by using a feedback shift register with appropriate
   connections specified by the elements of gg[], which was generated above.
   Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)          */
 {
   register int i,j ;
   int feedback ;

   for (i=0; i<nn-kk; i++)   bb[i] = 0 ;
   for (i=kk-1; i>=0; i--)
    {  feedback = data[i]^bb[nn-kk-1] ;
       if (feedback != 0) /* feedback term is non-zero */
        { for (j=nn-kk-1; j>0; j--)
              bb[j] = bb[j-1]^galois(alpha_to[gg[j]],feedback) ;
          bb[0] = galois(alpha_to[gg[0]],feedback) ;
        }
       else /* feedback term is zero. encoder becomes a single-byte shifter */
        { for (j=nn-kk-1; j>0; j--)
            bb[j] = bb[j-1] ;
          bb[0] = 0 ;
        } ;
    } ;
 }


/* Performs ERRORS-ONLY decoding of RS codes. If decoding is successful,
 writes the codeword into recd[] itself. Otherwise recd[] is unaltered. 
 If channel caused no more than "tt" errors, the tranmitted codeword will
 be recovered.
*/ 

int decode_rs()
 {
   register int i,j,u,q ;
   int elp[nn-kk+2][nn-kk], d[nn-kk+2], l[nn-kk+2], u_lu[nn-kk+2], s[nn-kk+1] ;
   int count=0, syn_error=0, root[tt], loc[tt], z[tt+1], err[nn], reg[tt+1] ;

/* recd[] is in polynomial form, convert to index form */
   for (i=0;i < nn;i++) recd[i] = index_of[recd[i]];

/* first form the syndromes; i.e., evaluate recd(x) at roots of g(x) namely
 @**(b0+i), i = 0, ... ,(2*tt-1) */
   for (i=1; i <= nn-kk; i++)
    { s[i] = 0 ;
      for (j=0; j < nn; j++)
        if (recd[j] != -1)
          s[i] ^= alpha_to[(recd[j]+(b0+i-1)*j)%nn] ;      /* recd[j] in index form */
/* convert syndrome from polynomial form to index form  */
      if (s[i] != 0)  syn_error = 1 ;   /* set flag if non-zero syndrome => error */
      s[i] = index_of[s[i]] ;
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
      d[0] = 0 ;           /* index form */
      d[1] = s[1] ;        /* index form */
      elp[0][0] = 0 ;      /* index form */
      elp[1][0] = 1 ;      /* polynomial form */
      for (i=1; i<nn-kk; i++)
        { elp[0][i] = -1 ;   /* index form */
          elp[1][i] = 0 ;   /* polynomial form */
        }
      l[0] = 0 ;
      l[1] = 0 ;
      u_lu[0] = -1 ;
      u_lu[1] = 0 ;
      u = 0 ;

      do
      {
        u++ ;
        if (d[u]==-1)
          { l[u+1] = l[u] ;
            for (i=0; i<=l[u]; i++)
             {  elp[u+1][i] = elp[u][i] ;
                elp[u][i] = index_of[elp[u][i]] ;
             }
          }
        else
/* search for words with greatest u_lu[q] for which d[q]!=0 */
          { q = u-1 ;
            while ((d[q]==-1) && (q>0)) q-- ;
/* have found first non-zero d[q]  */
            if (q>0)
             { j=q ;
               do
               { j-- ;
                 if ((d[j]!=-1) && (u_lu[q]<u_lu[j]))
                   q = j ;
               }while (j>0) ;
             } ;

/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
/* store degree of new elp polynomial */
            if (l[u]>l[q]+u-q)  l[u+1] = l[u] ;
            else  l[u+1] = l[q]+u-q ;

/* form new elp(x) */
            for (i=0; i<nn-kk; i++)    elp[u+1][i] = 0 ;
            for (i=0; i<=l[q]; i++)
              if (elp[q][i]!=-1)
                elp[u+1][i+u-q] = alpha_to[(d[u]+nn-d[q]+elp[q][i])%nn] ;
            for (i=0; i<=l[u]; i++)
              { elp[u+1][i] ^= elp[u][i] ;
                elp[u][i] = index_of[elp[u][i]] ;  /*convert old elp value to index*/
              }
          }
        u_lu[u+1] = u-l[u+1] ;

/* form (u+1)th discrepancy */
        if (u<nn-kk)    /* no discrepancy computed on last iteration */
          {
            if (s[u+1]!=-1)
                   d[u+1] = alpha_to[s[u+1]] ;
            else
              d[u+1] = 0 ;
            for (i=1; i<=l[u+1]; i++)
              if ((s[u+1-i]!=-1) && (elp[u+1][i]!=0))
                d[u+1] ^= alpha_to[(s[u+1-i]+index_of[elp[u+1][i]])%nn] ;
            d[u+1] = index_of[d[u+1]] ;    /* put d[u+1] into index form */
          }
      } while ((u < nn-kk) && (l[u+1] <= tt)) ;


      u++ ;
      if (l[u] <= tt)         /* can correct error */
       {
/* put elp into index form */
         for (i=0; i<=l[u]; i++)   elp[u][i] = index_of[elp[u][i]] ;

/* find roots of the error location polynomial */
         for (i=1; i <= l[u]; i++)
           reg[i] = elp[u][i] ;
         count = 0 ;
         for (i=1; i <= nn; i++)
          {  q = 1 ;
             for (j=1; j<=l[u]; j++)
              if (reg[j]!=-1)
                { reg[j] = (reg[j]+j)%nn ;
                  q ^= alpha_to[reg[j]] ;
                } ;
             if (!q)        /* store root and error location number indices */
              { root[count] = i;
                loc[count] = nn-i ;
                count++ ;
              };
          } ;
         if (count == l[u])    /* no. roots = degree of elp hence <= tt errors */
          {
/* form polynomial z(x) */
           for (i=1; i <= l[u]; i++)        /* Z[0] = 1 always - do not need */
            { if ((s[i]!=-1) && (elp[u][i]!=-1))
                 z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]] ;
              else if ((s[i]!=-1) && (elp[u][i]==-1))
                      z[i] = alpha_to[s[i]] ;
                   else if ((s[i]==-1) && (elp[u][i]!=-1))
                          z[i] = alpha_to[elp[u][i]] ;
                        else
                          z[i] = 0 ;
              for (j=1; j<i; j++)
                if ((s[j]!=-1) && (elp[u][i-j]!=-1))
                   z[i] ^= alpha_to[(elp[u][i-j] + s[j])%nn] ;
              z[i] = index_of[z[i]] ;         /* put into index form */
            } ;

  /* evaluate errors at locations given by error location numbers loc[i] */
           for (i=0; i<nn; i++)
             { err[i] = 0 ;
               if (recd[i]!=-1)        /* convert recd[] to polynomial form */
                 recd[i] = alpha_to[recd[i]] ;
               else  recd[i] = 0 ;
             }
           for (i=0; i < l[u]; i++)    /* compute numerator of error term first */
            { err[loc[i]] = 1;       /* accounts for z[0] */
              for (j=1; j<=l[u]; j++){
                if (z[j]!=-1)
                  err[loc[i]] ^= alpha_to[(z[j]+j*root[i])%nn] ;
	      } /* z(x) evaluated at X(l)**(-1) */
              if (err[loc[i]] != 0) /* term X(l)**(1-b0) */
		 err[loc[i]] = alpha_to[(index_of[err[loc[i]]]+root[i]*(b0+nn-1))%nn];
              if (err[loc[i]]!=0)
               {
		 err[loc[i]] = index_of[err[loc[i]]] ;
                 q = 0 ;     /* form denominator of error term */
                 for (j=0; j<l[u]; j++)
                   if (j!=i)
                     q += index_of[1^alpha_to[(loc[j]+root[i])%nn]] ;
                 q = q % nn ;
                 err[loc[i]] = alpha_to[(err[loc[i]]-q+nn)%nn] ;
                 recd[loc[i]] ^= err[loc[i]] ;  /*recd[i] must be in polynomial form */
               }
            }
	    return(1);
          }
          else{    /* no. roots != degree of elp => >tt errors and cannot solve */
           	for (i=0; i<nn; i++){        /* could return error flag if desired */
               	   if (recd[i]!=-1)        /* convert recd[] to polynomial form */
                 	recd[i] = alpha_to[recd[i]] ;
                   else  
			recd[i] = 0 ;     /* just output received word as is */
		}
		return(2);
	  }
       }
      else{         /* elp has degree has degree >tt hence cannot solve */
       	for (i=0; i<nn; i++){       /* could return error flag if desired */
            if (recd[i]!=-1)        /* convert recd[] to polynomial form */
                recd[i] = alpha_to[recd[i]] ;
            else  
		recd[i] = 0 ;     /* just output received word as is */
	}
	return(3);
      }
   }
   else{       /* no non-zero syndromes => no errors: output received codeword */
       	for (i=0; i < nn; i++){
       	   if (recd[i] != -1)        /* convert recd[] to polynomial form */
         	recd[i] = alpha_to[recd[i]] ;
           else  
	 	recd[i] = 0 ;
 	}
	return(0);
   }
 }

/* 
    Generates the Galois field and then the generator polynomial.
  Must be recompiled for each different generator polynomial after 
  setting parameters at the top of this file. 
*/

int main()
{
  int i,j,k,l;
  int no_encoder_errors;
  int error_status[nn];

  // Initialisation: generation du champ de Galois
  printf("Generation du champ de Galois GF(%d)\n\n",n1); 
  generate_gf();
  gen_poly();


#ifdef DEBUG
  // Deboguage pour s'assurer que l'encodeur est bien
  // configure selon le standard WiMAX

  printf("\n g(x) du code RS (%d,%d) pouvant corriger %d erreurs: \n",nn,kk,tt);
  printf("Le coefficient est l'exposant de @ = element primitif\n");
  for (i=0;i <= nn-kk ;i++){
      printf("%d x^%d ",gg[i],i);
      if (i < nn-kk) printf(" + ");
      if (i && (i % 7 == 0)) printf("\n"); /* 8 coefficients per line */
  }
  printf("\n");

  printf("\n Le coefficient est la representation en base {@^%d,...,@,1}\n",mm-1);
  for (i=0;i <= nn-kk ;i++){
      printf("%#x x^%d ",alpha_to[gg[i]],i);
      if (i < nn-kk) printf(" + ");
      if (i && (i % 7 == 0)) printf("\n"); /* 8 coefficients per line */
  }
  printf("\n\n");
#endif
  
  // Envoi des vecteurs de tests aleatoires
  #define NB_VECTORS 100
  srand(time(NULL));
  for (i=0;i <= tt;i++) {
      printf("Envoi de %d vecteurs d'octets ayant chacun %d erreurs: ", NB_VECTORS, i);
      no_encoder_errors = 0;
      for (j=0;j < NB_VECTORS;j++) {
          
          // Creation d'un vecteur de test aleatoire
          for (k=0;k < kk;k++) data[k] = (int) (rand() % 256);

          // Encodage Reed-Solomon
          encode_rs();

          // Recuperation des octets redondants produits par l'encodeur
          for (k=0;k < nn-kk;k++) {
              recd[kk+k] = bb[k];
          }

          // Recuperation des octets envoyes
          for (k=0;k < kk;k++) {
              recd[k] = data[k];
          }

          // Ajout de i erreurs au vecteur de donnees recu
          // (Simulation de i erreurs produites par le canal
          // de communication)
          for (k=0;k < nn;k++) error_status[k] = 0;
          for (k=0;k < i; k++) {
              // Selection aleatoire de i indices pour y injecter
              // une erreur
              int error_index = (int) (rand() % 256);
              if(error_status[error_index] == 0) {
                  // Injection d'une erreur a l'indice choisi
                  recd[error_index] = (recd[error_index] ^ rand()) % 256;
                  error_status[error_index] = 1;
              } else {
                  k--;
              }
          }

          // Decodage Reed-Solomon: si les donnees avaient ete
          // encodees correctement, alors il est posssible de
          // recuperer les donnees initiales malgre l'injection
          // de i erreurs.
          decode_rs();

          // Comparaison des donnees decodees avec les donnees
          // avec les donnes envoyees
          for (k=0;k < kk;k++) {
              if(data[k] != recd[k]) {
#ifdef DEBUG
                  printf("\nErreur: valeur initiale etait de:\n");
                  for(l=0;l < kk;l++) {
                     printf("%#x ", data[l]);
                  }
                  printf("\nValeur recue est de:\n");
                  for(l=0;l < kk;l++) {
                     printf("%#x ", recd[nn-kk+l]);
                  }
#endif
                  // Les donnees ont ete mal encodees
                  no_encoder_errors++;
                  break;
              }
          }
      }
      if(no_encoder_errors == 0) {
          printf("OK\n");
      } else {
          printf("%d erreurs sur %d vecteurs.\n", no_encoder_errors, NB_VECTORS);
      }
  }
  #undef NB_VECTORS

  return 0;
}

