#include <cstring> // for memset
#include "RandomNumberSequenceGen.hpp"

void MTRandomNumberSequenceGen::SeedGen(unsigned int s) {
	memset(state, 0, N * sizeof(unsigned int));

	index = N;
	state[0] = s & 0xFFFFFFFFUL; // for > 32 bit machines

	for (unsigned int n = 1; n < N; ++n) {
		state[n] = 1812433253UL * (state[n - 1] ^ (state[n - 1] >> 30)) + n;
		state[n] &= 0xFFFFFFFFUL; // for > 32 bit machines
	}
}

void MTRandomNumberSequenceGen::GenState() { 
	index = 0;

	for (unsigned int n = 0; n < (N - M); ++n) {
		state[n] = state[n + M] ^ Twiddle(state[n], state[n + 1]);
	}
	for (unsigned int n = N - M; n < (N - 1); ++n) {
		state[n] = state[n + M - N] ^ Twiddle(state[n], state[n + 1]);
	}

	state[N - 1] = state[M - 1] ^ Twiddle(state[N - 1], state[0]);
}
