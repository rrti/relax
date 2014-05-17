#ifndef RELAX_RANDOM_NUMBER_SEQUENCE_GEN_HDR
#define RELAX_RANDOM_NUMBER_SEQUENCE_GEN_HDR

#include "INumberSequenceGen.hpp"

// Mersenne-Twister RNG: generates uniformly distributed 32-bit
// random integers and normalized double-precision floating-point
// numbers in the half-open interval [0, 1)
class MTRandomNumberSequenceGen: public INumberSequenceGen {
public:
	// default constructor: uses default seed
	MTRandomNumberSequenceGen() { SeedGen(5489UL); }
	MTRandomNumberSequenceGen(unsigned int s) { SeedGen(s); }
	MTRandomNumberSequenceGen(const MTRandomNumberSequenceGen& rng) { *this = rng; }

	MTRandomNumberSequenceGen& operator = (const MTRandomNumberSequenceGen& rng) {
		for (unsigned int n = 0; n < N; n++) {
			state[n] = rng.state[n];
		}
		index = rng.index;
		return *this;
	}

	unsigned int NextInt() { return GenNextValue(); }
	double NextFlt() { return (GenNextValue() / RNG_MAX_VALUE); }

private:
	unsigned int GenNextValue();
	unsigned int Twiddle(unsigned int u, unsigned int v) const {
		return (((u & 0x80000000UL) | (v & 0x7FFFFFFFUL)) >> 1) ^ ((v & 1UL)? 0x9908B0DFUL: 0x0UL);
	}

	void SeedGen(unsigned int);
	void GenState();

private:
	// compile time constants
	static const unsigned int N = 624;
	static const unsigned int M = 397;

	// note: (1 << (32-1)) * 2.0 because (1 << 32) would overflow the intermediary uint
	static const double RNG_MAX_VALUE = (1U << ((sizeof(unsigned int) * 8U) - 1)) * 2.0;

	// state vector
	unsigned int state[N];
	// current state index
	unsigned int index;
};



inline unsigned int MTRandomNumberSequenceGen::GenNextValue() {
	if (index == N) {
		// new state vector needed, reseed
		GenState();
	}

	// GenState() is split off to be non-inline, because it is
	// only called once in every <N> calls and otherwise this
	// function would probably become too big to get inlined
	unsigned int x = state[index++];

	x ^= (x >> 11);
	x ^= (x << 7) & 0x9D2C5680UL;
	x ^= (x << 15) & 0xEFC60000UL;

	return (x ^ (x >> 18));
}

#endif
