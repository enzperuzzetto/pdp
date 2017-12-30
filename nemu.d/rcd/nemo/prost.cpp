
// prost (probas and stats)
// Copyright (C) 2012 Damien Magoni. All rights reserved.

#include <cmath>
#include "mt19937.h"
#include "prost.h"

namespace prost
{
	// return the nearest int of d
	long round_to_l(double d)
	{
		double n = 0;
		if ( fabs(modf(d, &n)) <= 0.5 )
			return static_cast<long>(n);
		else if ( d > 0 )
			return static_cast<long>(n) + 1;
		else
			return static_cast<long>(n) - 1;
	}

	// proba must be expressed in per cent (e.g. 37.4 %)
	bool roll(double pc_probability)
	{
		if ( random_l(1, 1000000000) <= round_to_l(10000000 * pc_probability) ) // precision = 7 digits after the point
			return true;
		else
			return false;
	}

	long random_l(long inf, long sup)
	{
		// Classic ANSI C rand() LC-PRNG
		// put next instruction at the beginning of the program
		// srand((unsigned)time(0));
		// return round_to_int((sup - inf) * static_cast<double>(rand()) / RAND_MAX) + inf;

		// Mersenne Twister GFSR-PRNG
		// put next instruction at the beginning of the program
		// init_genrand((unsigned long)s);
		return round_to_l((sup - inf) * static_cast<double>(mt19937::genrand_int32()) / 0xFFFFFFFFUL) + inf;
	}
}
