#include <cstdio>
#include "../Defines.hpp"

#ifdef RELAX_POWERSET_TEST
#include "PowerSet.hpp"

int main() {
	std::set<int> S;
	std::vector< std::vector< std::set<int> > > PS;

	for (unsigned int n = 0; n < 6; n++) {
		S.insert(n + 1);
	}

	PowerSet(S, 2, PS);

	// for all subsets of size <0, ..., n>
	for (unsigned int n = 0; n < PS.size(); n++) {
		const std::vector<std::set<int> >& V = PS[n];

		printf("[n: %u]\n", n);

		// for all <k> subsets of size <n>
		for (unsigned int k = 0; k < V.size(); k++) {
			const std::set<int>& K = V[k];

			printf("\t[k: %u] = {", k + 1);

			// all elements of the <k>-th subset of size <n>
			for (std::set<int>::const_iterator it = K.begin(); it != K.end(); ++it) {
				std::set<int>::const_iterator iit = it;

				printf("%d", *it);

				if (++iit != K.end()) {
					printf(", ");
				}
			}

			printf("}\n");
		}
	}

	return 0;
}

#endif
