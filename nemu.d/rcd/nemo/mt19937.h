
// Mersenne Twister 19937

#if !defined(MT19937_H)
#define MT19937_H

namespace mt19937
{
	/* Period parameters */  
	const int N = 624;
	const int M = 397;
	const unsigned long int MATRIX_A = 0x9908b0dfUL;   /* constant vector a */
	const unsigned long int UPPER_MASK = 0x80000000UL; /* most significant w-r bits */
	const unsigned long int LOWER_MASK = 0x7fffffffUL; /* least significant r bits */

	void init_genrand(unsigned long int s);
	void init_by_array(unsigned long int init_key[], unsigned long int key_length);
	unsigned long int genrand_int32(void);
	long int genrand_int31(void);
	double genrand_real1(void);
	double genrand_real2(void);
	double genrand_real3(void);
	double genrand_res53(void);
}

#endif
