/*
 * Merlin++: C++ Class Library for Charged Particle Accelerator Simulations
 * Copyright (c) 2001-2018 The Merlin++ developers
 * This file is covered by the terms the GNU GPL version 2, or (at your option) any later version, see the file COPYING
 * This file is derived from software bearing the copyright notice in merlin4_copyright.txt
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>

#include "ACG.h"
using namespace std;

/*
 * Not a test in itself, but use the random number generator to output a
 * stream of random bytes, for testing with dieharder (or similar)
 *
 * ./MerlinTests/ScatteringTests/rand_test 1 | dieharder -a -g 200
 *
 * dieharder runs a number of tests on the stream, and reports on
 * deficiencies in their randomness.
 *
 */

int main(int argc, char* argv[])
{
	unsigned int seed = 0;

	if(argc >= 2)
	{
		seed = atoi(argv[1]);
	}
	cerr << "Random Seed: " << seed << endl;
	//RandomNG::init(seed);
	//RandomNG* rng = new RandomNG(seed);

	ACG rng(seed, 100);

	while(1)
	{
		int x = rng.asLong();
		cout.write((char *) &x, sizeof(x));
	}

}
