// This may look like C code, but it is really -*- C++ -*-
/*
Copyright (C) 1989 Free Software Foundation

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "ACG.h"
#include <assert.h>

/**
*   \class ACG
*
*	This is an extension of the older implementation of Algorithm M
*	which I previously supplied. The main difference between this
*	version and the old code are:
*
*		+ Andres searched high & low for good constants for
*		  the LCG.
*
*		+ there's more bit chopping going on.
*
*	The following contains his comments.
*
*	agn@UNH.CS.CMU.EDU sez..
*
*	The generator below is based on 2 well known
*	methods: Linear Congruential (LCGs) and Additive
*	Congruential generators (ACGs).
*
*	The LCG produces the longest possible sequence
*	of 32 bit random numbers, each being unique in
*	that sequence (it has only 32 bits of state).
*	It suffers from 2 problems: a) Independence
*	isn't great, that is the (n+1)th number is
*	somewhat related to the preceding one, unlike
*	flipping a coin where knowing the past outcomes
*	don't help to predict the next result.  b)
*	Taking parts of a LCG generated number can be
*	quite non-random: for example, looking at only
*	the least significant byte gives a permuted
*	8-bit counter (that has a period length of only
*	256).  The advantage of an LCA is that it is
*	perfectly uniform when run for the entire period
*	length (and very uniform for smaller sequences
*	too, if the parameters are chosen carefully).
*
*	ACGs have extremely long period lengths and
*	provide good independence.  Unfortunately,
*	uniformity isn't not too great. Furthermore, I
*	didn't find any theoretically analysis of ACGs
*	that addresses uniformity.
*
*	The RNG given below will return numbers
*	generated by an LCA that are permuted under
*	control of a ACG. 2 permutations take place: the
*	4 bytes of one LCG generated number are
*	subjected to one of 16 permutations selected by
*	4 bits of the ACG. The permutation a such that
*	byte of the result may come from each byte of
*	the LCG number. This effectively destroys the
*	structure within a word. Finally, the sequence
*	of such numbers is permuted within a range of
*	256 numbers. This greatly improves independence.
*
*
*  Algorithm M as describes in Knuths "Art of Computer Programming",
*	Vol 2. 1969
*  is used with a linear congruential generator (to get a good uniform
*  distribution) that is permuted with a Fibonacci additive congruential
*  generator to get good independence.
*
*  Bit, byte, and word distributions were extensively tested and pass
*  Chi-squared test near perfect scores (>7E8 numbers tested, Uniformity
*  assumption holds with probability > 0.999)
*
*  Run-up tests for on 7E8 numbers confirm independence with
*  probability > 0.97.
*
*  Plotting random points in 2d reveals no apparent structure.
*
*  Autocorrelation on sequences of 5E5 numbers (A(i) = SUM X(n)*X(n-i),
*	i=1..512)
*  results in no obvious structure (A(i) ~ const).
*
*  Except for speed and memory requirements, this generator outperforms
*  random() for all tests. (random() scored rather low on uniformity tests,
*  while independence test differences were less dramatic).
*
*  AGN would like to..
*  thanks to M.Mauldin, H.Walker, J.Saxe and M.Molloy for inspiration & help.
*
*  And I would (DGC) would like to thank Donald Knuth for AGN for letting me
*  use his extensions in this implementation.
*
*/

//
//	Part of the table on page 28 of Knuth, vol II. This allows us
//	to adjust the size of the table at the expense of shorter sequences.
//

static int randomStateTable[][3] =
{
	{3,7,16}, {4,9, 32}, {3,10, 32}, {1,11, 32}, {1,15,64}, {3,17,128},
	{7,18,128}, {3,20,128}, {2,21, 128}, {1,22, 128}, {5,23, 128}, {3,25, 128},
	{2,29, 128}, {3,31, 128}, {13,33, 256}, {2,35, 256}, {11,36, 256},
	{14,39,256}, {3,41,256}, {9,49,256}, {3,52,256}, {24,55,256}, {7,57, 256},
	{19,58,256}, {38,89,512}, {17,95,512}, {6,97,512}, {11,98,512}, {-1,-1,-1}
};

//
// spatial permutation table
//	RANDOM_PERM_SIZE must be a power of two
//

#define RANDOM_PERM_SIZE 64
_G_uint32_t randomPermutations[RANDOM_PERM_SIZE] =
{
	0xffffffff, 0x00000000,  0x00000000,  0x00000000,  // 3210
	0x0000ffff, 0x00ff0000,  0x00000000,  0xff000000,  // 2310
	0xff0000ff, 0x0000ff00,  0x00000000,  0x00ff0000,  // 3120
	0x00ff00ff, 0x00000000,  0xff00ff00,  0x00000000,  // 1230

	0xffff0000, 0x000000ff,  0x00000000,  0x0000ff00,  // 3201
	0x00000000, 0x00ff00ff,  0x00000000,  0xff00ff00,  // 2301
	0xff000000, 0x00000000,  0x000000ff,  0x00ffff00,  // 3102
	0x00000000, 0x00000000,  0x00000000,  0xffffffff,  // 2103

	0xff00ff00, 0x00000000,  0x00ff00ff,  0x00000000,  // 3012
	0x0000ff00, 0x00000000,  0x00ff0000,  0xff0000ff,  // 2013
	0x00000000, 0x00000000,  0xffffffff,  0x00000000,  // 1032
	0x00000000, 0x0000ff00,  0xffff0000,  0x000000ff,  // 1023

	0x00000000, 0xffffffff,  0x00000000,  0x00000000,  // 0321
	0x00ffff00, 0xff000000,  0x00000000,  0x000000ff,  // 0213
	0x00000000, 0xff000000,  0x0000ffff,  0x00ff0000,  // 0132
	0x00000000, 0xff00ff00,  0x00000000,  0x00ff00ff   // 0123
};

//
//	SEED_TABLE_SIZE must be a power of 2
//
#define SEED_TABLE_SIZE 32
static _G_uint32_t seedTable[SEED_TABLE_SIZE] =
{
	0xbdcc47e5, 0x54aea45d, 0xec0df859, 0xda84637b,
	0xc8c6cb4f, 0x35574b01, 0x28260b7d, 0x0d07fdbf,
	0x9faaeeb0, 0x613dd169, 0x5ce2d818, 0x85b9e706,
	0xab2469db, 0xda02b0dc, 0x45c60d6e, 0xffe49d10,
	0x7224fea3, 0xf9684fc9, 0xfc7ee074, 0x326ce92a,
	0x366d13b5, 0x17aaa731, 0xeb83a675, 0x7781cb32,
	0x4ec7c92d, 0x7f187521, 0x2cf346b4, 0xad13310f,
	0xb89cff2b, 0x12164de1, 0xa865168d, 0x32b56cdf
};

//
//	The LCG used to scramble the ACG
//
//
// LC-parameter selection follows recommendations in
// "Handbook of Mathematical Functions" by Abramowitz & Stegun 10th, edi.
//
// LC_A = 251^2, ~= sqrt(2^32) = 66049
// LC_C = result of a long trial & error series = 3907864577
//

static const _G_uint32_t LC_A = 66049;
static const _G_uint32_t LC_C = 3907864577;
static inline _G_uint32_t LCG(_G_uint32_t x)
{
	return( x * LC_A + LC_C );
}


ACG::ACG(_G_uint32_t seed, int size)
{
	int l;
	initialSeed = seed;

	//
	//	Determine the size of the state table
	//

	for (l = 0;
	        randomStateTable[l][0] != -1 && randomStateTable[l][1] < size;
	        l++);

	if (randomStateTable[l][1] == -1)
	{
		l--;
	}

	initialTableEntry = l;

	stateSize = randomStateTable[ initialTableEntry ][ 1 ];
	auxSize = randomStateTable[ initialTableEntry ][ 2 ];

	//
	//	Allocate the state table & the auxiliary table in a single malloc
	//

	state = new _G_uint32_t[stateSize + auxSize];
	auxState = &state[stateSize];

	reset();
}

//
//	Initialize the state
//
void
ACG::reset()
{
	_G_uint32_t u;

	if (initialSeed < SEED_TABLE_SIZE)
	{
		u = seedTable[ initialSeed ];
	}
	else
	{
		u = initialSeed ^ seedTable[ initialSeed & (SEED_TABLE_SIZE-1) ];
	}


	j = randomStateTable[ initialTableEntry ][ 0 ] - 1;
	k = randomStateTable[ initialTableEntry ][ 1 ] - 1;

	int i;
	for(i = 0; i < stateSize; i++)
	{
		state[i] = u = LCG(u);
	}

	for (i = 0; i < auxSize; i++)
	{
		auxState[i] = u = LCG(u);
	}

	k = u % stateSize;
	int tailBehind = (stateSize - randomStateTable[ initialTableEntry ][ 0 ]);
	j = k - tailBehind;
	if (j < 0)
	{
		j += stateSize;
	}

	lcgRecurr = u;

	assert(sizeof(double) == 2 * sizeof(unsigned int));
}

ACG::~ACG()
{
	if (state)
	{
		delete state;
	}
	state = nullptr;
	// don't delete auxState, it's really an alias for state.
}

//
//	Returns 32 bits of random information.
//

_G_uint32_t
ACG::asLong()
{
	_G_uint32_t result = state[k] + state[j];
	state[k] = result;
	j = (j <= 0) ? (stateSize-1) : (j-1);
	k = (k <= 0) ? (stateSize-1) : (k-1);

	short int auxIndex = (result >> 24) & (auxSize - 1);
	_G_uint32_t auxACG = auxState[auxIndex];
	auxState[auxIndex] = lcgRecurr = LCG(lcgRecurr);

	//
	// 3c is a magic number. We are doing four masks here, so we
	// do not want to run off the end of the permutation table.
	// This insures that we have always got four entries left.
	//
	_G_uint32_t *perm = & randomPermutations[result & 0x3c];

	result =  *(perm++) & auxACG;
	result |= *(perm++) & ((auxACG << 24)
	                       | ((auxACG >> 8)& 0xffffff));
	result |= *(perm++) & ((auxACG << 16)
	                       | ((auxACG >> 16) & 0xffff));
	result |= *(perm++) & ((auxACG <<  8)
	                       | ((auxACG >> 24) &   0xff));

	return(result);
}
