
// probas and stats (prost)
// Copyright (C) 2012-2016 Damien Magoni

#include <cmath>
#include "mt19937.hpp"
#include "prost.hpp"

namespace prost
{
	// return the nearest int of d
	int round_to_int(double d)
	{
		double n = 0;
		if ( fabs(modf(d, &n)) <= 0.5 )
			return static_cast<int>(n);
		else if ( d > 0 )
			return static_cast<int>(n) + 1;
		else
			return static_cast<int>(n) - 1;
	}

	// proba must be expressed in per cent (e.g. 37.4 %)
	bool roll(double pc_probability)
	{
		if ( random(1, 1000000000) <= round_to_int(10000000 * pc_probability) ) // precision = 7 digits after the point
			return true;
		else
			return false;
	}

	int random(int inf, int sup)
	{
		// Classic ANSI C rand() LC-PRNG
		// put next instruction at the beginning of the program
		// srand((unsigned)time(0));
		// return round_to_int((sup - inf) * static_cast<double>(rand()) / RAND_MAX) + inf;

		// Mersenne Twister GFSR-PRNG
		// put next instruction at the beginning of the program
		// init_genrand((unsigned long)s);
		return round_to_int((sup - inf) * static_cast<double>(mt19937::genrand_int32()) / 0xFFFFFFFFUL) + inf;
	}

	double random_dbl(double inf, double sup)
	{
		return (sup - inf) * (static_cast<double>(mt19937::genrand_int32()) / static_cast<double>(0xFFFFFFFFUL)) + inf;
	}
}
