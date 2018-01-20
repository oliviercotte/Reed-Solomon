
package reed_solomon;

parameter CORRECTION_CAPACITY = 8; // tt mm 8 symbole can be corrected
parameter BLOCK_LENGTH = 255; // n = 2**m - 1 = 2**8 - 1 = 255
parameter MESSAGE_LENGTH = ( BLOCK_LENGTH - 2 * CORRECTION_CAPACITY); // (k) message length < block length (n)
parameter NB_SYMBOL_BITS = 8; // m /* RS code over GF(2**mm) - change to suit */ // GF(q) q = 2**m = 2**8 , n = q ou n = q-1
parameter b0 = 0;           /* g(x) has roots @**b0, @**(b0+1), ... ,@^(b0+2*tt-1) */

static int alpha_to[BLOCK_LENGTH + 1];
static int index_of[BLOCK_LENGTH + 1];
static int gg[BLOCK_LENGTH - MESSAGE_LENGTH + 1];

/**** Primitive polynomial  1+x^2+x^3+x^4+x^8 ****/
static int pp [NB_SYMBOL_BITS+1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1}; 

function int galois(int in1, int in2);

	int A0,A1,A2,A3,A4,A5,A6,A7,B0,B1,B2,B3,B4,B5,B6,B7,Z0,Z1,Z2,Z3,Z4,Z5,Z6,Z7;
      	int Z;
	
	A0= in1 & 16'h1;
	B0= in2 & 16'h1;
	A1= (in1 >> 1) & 16'h1;
	B1= (in2 >> 1) & 16'h1;
	A2= (in1 >> 2) & 16'h1;
	B2= (in2 >> 2) & 16'h1;
	A3= (in1 >> 3) & 16'h1;
	B3= (in2 >> 3) & 16'h1;
	A4= (in1 >> 4) & 16'h1;
	B4= (in2 >> 4) & 16'h1;
	A5= (in1 >> 5) & 16'h1;
	B5= (in2 >> 5) & 16'h1;
	A6= (in1 >> 6) & 16'h1;
	B6= (in2 >> 6) & 16'h1;
	A7= (in1 >> 7) & 16'h1;
	B7= (in2 >> 7) & 16'h1;
	
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

endfunction : galois

// Genere les champs de galoie / corps de galoie / corps comutatif
function void generate_gf(); 
	
	int mask;
	mask = 1;

	//mask = 1;
	alpha_to[NB_SYMBOL_BITS] = 0;
	alpha_to[BLOCK_LENGTH] = 0;

	for (int i = 0; i < NB_SYMBOL_BITS; i++) 
	begin
		alpha_to[i] = mask;
		index_of[alpha_to[i]] = i;

		/* If pp[i] == 1 then, term @^i occurs in poly-repr of @^mm */
		if (pp[i] != 0) begin 
			/* Bit-wise EXOR operation */
			alpha_to[NB_SYMBOL_BITS] ^= mask;  
		end

		/* single left-shift */
		mask <<= 1; 

	end // for

	index_of[alpha_to[NB_SYMBOL_BITS]] = NB_SYMBOL_BITS;

	/* Have obtained poly-repr of @^mm. Poly-repr of @^(i+1) is given by
	poly-repr of @^i shifted left one-bit and accounting for any @^mm
	term that may occur when poly-repr of @^i is shifted. */
	mask >>= 1;

	for (int i = NB_SYMBOL_BITS + 1; i < BLOCK_LENGTH; i++) 
	begin
		if (alpha_to[i - 1] >= mask) begin
			alpha_to[i] = alpha_to[NB_SYMBOL_BITS] ^ ((alpha_to[i - 1] ^ mask) << 1);
		end
		else begin
			alpha_to[i] = alpha_to[i - 1] << 1;
		end

		index_of[alpha_to[i]] = i;
	end //for
	index_of[0] = -1;

endfunction : generate_gf

function void generate_poly();
/* Obtain the generator polynomial of the tt-error correcting, length
nn=(2**mm -1) Reed Solomon code from the product of (X+@**(b0+i)), i = 0, ... ,(2*tt-1)
Examples: 	If b0 = 1, tt = 1. deg(g(x)) = 2*tt = 2.
g(x) = (x+@) (x+@**2)
If b0 = 0, tt = 2. deg(g(x)) = 2*tt = 4.
g(x) = (x+1) (x+@) (x+@**2) (x+@**3)
*/
	gg[0] = alpha_to[b0];
	gg[1] = 1;    /* g(x) = (X+@**b0) initially */

	for (int i = 2; i <= BLOCK_LENGTH - MESSAGE_LENGTH; i++)
	begin
		gg[i] = 1;
		/* Below multiply (gg[0]+gg[1]*x + ... +gg[i]x^i) by (@**(b0+i-1) + x) */
		for (int j = i - 1; j>0; j--) 
		begin
			if (gg[j] != 0) begin
				 gg[j] = gg[j - 1] ^ alpha_to[((index_of[gg[j]]) + b0 + i - 1) % BLOCK_LENGTH];
			end 
			else begin
				gg[j] = gg[j - 1];
			end

			gg[0] = alpha_to[((index_of[gg[0]]) + b0 + i - 1) % BLOCK_LENGTH];     /* gg[0] can never be zero */
		end
	end

	/* convert gg[] to index form for quicker encoding */
	for (int i = 0; i <= BLOCK_LENGTH - MESSAGE_LENGTH; i++)  begin
		gg[i] = index_of[gg[i]];
	end

endfunction : generate_poly

function automatic int decode(ref logic[7:0] msg[BLOCK_LENGTH]);

	int i, j, u, q;
	int elp[BLOCK_LENGTH - MESSAGE_LENGTH + 2][BLOCK_LENGTH - MESSAGE_LENGTH], d[BLOCK_LENGTH - MESSAGE_LENGTH + 2], l[BLOCK_LENGTH - MESSAGE_LENGTH + 2], u_lu[BLOCK_LENGTH - MESSAGE_LENGTH + 2], s[BLOCK_LENGTH - MESSAGE_LENGTH + 1];
	int count = 0, syn_error = 0, root[CORRECTION_CAPACITY], loc[CORRECTION_CAPACITY], z[CORRECTION_CAPACITY + 1], err[BLOCK_LENGTH], rr[CORRECTION_CAPACITY + 1];
	int msg_temp[BLOCK_LENGTH];
	/* msg[] is in polynomial form, convert to index form */
	for (i = 0; i < BLOCK_LENGTH; i++) begin
		msg_temp[i] = index_of[msg[i]];
	end



	/* first form the syndromes; i.e., evaluate msg(x) at roots of g(x) namely
	@**(b0+i), i = 0, ... ,(2*CORRECTION_CAPACITY-1) */
	for (i = 1; i <= BLOCK_LENGTH - MESSAGE_LENGTH; i++)
	 begin
		s[i] = 0;
		for (j = 0; j < BLOCK_LENGTH; j++)
		 begin
			if (msg_temp[j] != -1) begin
				s[i] ^= alpha_to[(msg_temp[j] + (b0 + i - 1)*j) % BLOCK_LENGTH];      /* msg[j] in index form */
			end
		 end
		/* convert syndrome from polynomial form to index form  */
		if (s[i] != 0)
		 begin
			syn_error = 1;   /* set flag if non-zero syndrome => error */
		 end
		s[i] = index_of[s[i]];
	 end;

	if (syn_error)       /* if errors, try and correct */
	 begin
		/* compute the error location polynomial via the Berlekamp iterative algorithm,
		following the terminology of Lin and Costello :   d[u] is the 'mu'th
		discrepancy, where u = 'mu'+ 1 and 'mu' (the Greek leCORRECTION_CAPACITYer!) is the step number
		ranging from -1 to 2*CORRECTION_CAPACITY (see L&C),  l[u] is the
		degree of the elp at that step, and u_l[u] is the difference between the
		step number and the degree of the elp.

		The notation is the same as that in Lin and Costello's book; pages 155-156 and 175.

		*/
		/* initialise table entries */
		d[0] = 0;           /* index form */
		d[1] = s[1];        /* index form */
		elp[0][0] = 0;      /* index form */
		elp[1][0] = 1;      /* polynomial form */
		for (i = 1; i<BLOCK_LENGTH - MESSAGE_LENGTH; i++)
		 begin
			elp[0][i] = -1;   /* index form */
			elp[1][i] = 0;   /* polynomial form */
		 end
		l[0] = 0;
		l[1] = 0;
		u_lu[0] = -1;
		u_lu[1] = 0;
		u = 0;

		do
		 begin
			u++;
			if (d[u] == -1)
			 begin
				l[u + 1] = l[u];
				for (i = 0; i <= l[u]; i++)
				 begin
					elp[u + 1][i] = elp[u][i];
					elp[u][i] = index_of[elp[u][i]];
				 end
			 end
			else
				/* search for words with greatest u_lu[q] for which d[q]!=0 */
			 begin
				q = u - 1;
				while ((d[q] == -1) && (q>0)) begin 
					q--;
				end
				/* have found first non-zero d[q]  */
				if (q>0)
				 begin
					j = q;
					do
					 begin
						j--;
						if ((d[j] != -1) && (u_lu[q]<u_lu[j])) begin
							q = j;
						end
					 end while (j>0);
				 end;

				/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
				/* store degree of new elp polynomial */
				if (l[u]>l[q] + u - q) begin
					l[u + 1] = l[u];
				end
				else  begin
					l[u + 1] = l[q] + u - q;
				end

				/* form new elp(x) */
				for (i = 0; i<BLOCK_LENGTH - MESSAGE_LENGTH; i++) begin
					elp[u + 1][i] = 0;
				end
				for (i = 0; i <= l[q]; i++)
				begin
					if (elp[q][i] != -1) begin
						elp[u + 1][i + u - q] = alpha_to[(d[u] + BLOCK_LENGTH - d[q] + elp[q][i]) % BLOCK_LENGTH];
					end
				end
				for (i = 0; i <= l[u]; i++)
				 begin
					elp[u + 1][i] ^= elp[u][i];
					elp[u][i] = index_of[elp[u][i]];  /*convert old elp value to index*/
				 end
			 end
			u_lu[u + 1] = u - l[u + 1];

			/* form (u+1)th discrepancy */
			if (u<BLOCK_LENGTH - MESSAGE_LENGTH)    /* no discrepancy computed on last iteration */
			 begin
				if (s[u + 1] != -1) begin
					d[u + 1] = alpha_to[s[u + 1]];
				end
				else begin
					d[u + 1] = 0;
				end
				for (i = 1; i <= l[u + 1]; i++) begin
					if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0)) begin
						d[u + 1] ^= alpha_to[(s[u + 1 - i] + index_of[elp[u + 1][i]]) % BLOCK_LENGTH];
					end
				end
				d[u + 1] = index_of[d[u + 1]];    /* put d[u+1] into index form */
			 end
		 end while ((u < BLOCK_LENGTH - MESSAGE_LENGTH) && (l[u + 1] <= CORRECTION_CAPACITY));


		u++;
		if (l[u] <= CORRECTION_CAPACITY)         /* can correct error */
		 begin
			/* put elp into index form */
			for (i = 0; i <= l[u]; i++) begin
				elp[u][i] = index_of[elp[u][i]];
			end

			/* find roots of the error location polynomial */
			for (i = 1; i <= l[u]; i++) begin
				rr[i] = elp[u][i];
			end

			count = 0;

			for (i = 1; i <= BLOCK_LENGTH; i++)
			 begin
				q = 1;
				for (j = 1; j <= l[u]; j++) begin
					if (rr[j] != -1)
					begin
						rr[j] = (rr[j] + j) % BLOCK_LENGTH;
						q ^= alpha_to[rr[j]];
					 end
				end
				if ( !q )        /* store root and error location number indices */
				 begin
					root[count] = i;
					loc[count] = BLOCK_LENGTH - i;
					count++;
				 end
			 end

			if (count == l[u])    /* no. roots = degree of elp hence <= CORRECTION_CAPACITY errors */
			 begin
				/* form polynomial z(x) */
				for (i = 1; i <= l[u]; i++)        /* Z[0] = 1 always - do not need */
				 begin
					if ((s[i] != -1) && (elp[u][i] != -1)) begin
						z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]];
					end
					else if ((s[i] != -1) && (elp[u][i] == -1)) begin
						z[i] = alpha_to[s[i]];
					end
					else if ((s[i] == -1) && (elp[u][i] != -1)) begin
						z[i] = alpha_to[elp[u][i]];
					end
					else begin
						z[i] = 0;
					end
					for (j = 1; j<i; j++) begin
						if ((s[j] != -1) && (elp[u][i - j] != -1)) begin
							z[i] ^= alpha_to[(elp[u][i - j] + s[j]) % BLOCK_LENGTH];
						end
					end
					z[i] = index_of[z[i]];         /* put into index form */
				 end;

				/* evaluate errors at locations given by error location numbers loc[i] */
				for (i = 0; i<BLOCK_LENGTH; i++)
				 begin
					err[i] = 0;
					if (msg_temp[i] != -1) begin        /* convert msg[] to polynomial form */
						msg[i] = alpha_to[msg_temp[i]];
					end
					else  begin
						msg[i] = 0;
					end
				 end
				for (i = 0; i < l[u]; i++)    /* compute numerator of error term first */
				 begin
					err[loc[i]] = 1;       /* accounts for z[0] */
					for (j = 1; j <= l[u]; j++) begin
						if (z[j] != -1) begin
							err[loc[i]] ^= alpha_to[(z[j] + j*root[i]) % BLOCK_LENGTH];
						end
					 end /* z(x) evaluated at X(l)**(-1) */
					if (err[loc[i]] != 0) begin/* term X(l)**(1-b0) */
						err[loc[i]] = alpha_to[(index_of[err[loc[i]]] + root[i] * (b0 + BLOCK_LENGTH - 1)) % BLOCK_LENGTH];
					end
					if (err[loc[i]] != 0)
					 begin
						err[loc[i]] = index_of[err[loc[i]]];
						q = 0;     /* form denominator of error term */
						for (j = 0; j<l[u]; j++) begin
							if (j != i) begin
								q += index_of[1 ^ alpha_to[(loc[j] + root[i]) % BLOCK_LENGTH]];
							end
						end
						q = q % BLOCK_LENGTH;
						err[loc[i]] = alpha_to[(err[loc[i]] - q + BLOCK_LENGTH) % BLOCK_LENGTH];
						msg[loc[i]] ^= err[loc[i]];  /*msg[i] must be in polynomial form */
					 end
				 end

				return 1;
			 end
			else begin    /* no. roots != degree of elp => >CORRECTION_CAPACITY errors and cannot solve */

				for (i = 0; i<BLOCK_LENGTH; i++) begin        /* could return error flag if desired */
					if (msg_temp[i] != -1) begin     /* convert msg[] to polynomial form */
						msg[i] = alpha_to[msg_temp[i]];
					end
					else begin
						msg[i] = 0;     /* just output received word as is */
					end
				 end

				return 2;
			 end
		 end
		else begin         /* elp has degree has degree >CORRECTION_CAPACITY hence cannot solve */
			for (i = 0; i<BLOCK_LENGTH; i++) begin       /* could return error flag if desired */
				if (msg_temp[i] != -1)  begin       /* convert msg[] to polynomial form */
					msg[i] = alpha_to[msg_temp[i]];
				end
				else begin
					msg[i] = 0;     /* just output received word as is */
				end
			 end

			return 3;
		 end
	 end
	else begin       /* no non-zero syndromes => no errors: output received codeword */
		for (i = 0; i < BLOCK_LENGTH; i++) begin
			if (msg_temp[i] != -1) begin     /* convert msg[] to polynomial form */
				msg[i] = alpha_to[msg_temp[i]];
			end
			else begin
				msg[i] = 0;
			end
		 end

		return 0;
	 end

	endfunction : decode

endpackage : reed_solomon
