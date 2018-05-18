/*
 * Merlin++: C++ Class Library for Charged Particle Accelerator Simulations
 * Copyright (c) 2001-2018 The Merlin++ developers
 * This file is covered by the terms the GNU GPL version 2, or (at your option) any later version, see the file COPYING
 * This file is derived from software bearing the copyright notice in merlin4_copyright.txt
 */

#ifndef _Landau_h_
#define _Landau_h_
#include "Random.h"

class Landau: public Random
{
//const float F[982];

public:
	Landau(RNG *gen);

	virtual double operator()();
};


inline Landau::Landau(RNG *gen) : Random(gen) {}

#endif