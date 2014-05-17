#ifndef RELAX_POWERSET_HDR
#define RELAX_POWERSET_HDR

#include <vector>
#include <set>

template<typename T> void PowerSet(
	const typename std::set<T>& S,
	const typename std::set< T>::const_iterator& SIt,
	typename std::set<T>& K,
	typename std::vector< std::vector< std::set<T> > >& PS,
	const unsigned int depth,
	const unsigned int maxDepth
) {
	if (depth > maxDepth) {
		return;
	}

	// make a copy of K
	PS[depth].push_back(K);

	for (typename std::set<T>::iterator it = SIt; it != S.end(); ++it) {
		typename std::set<T>::iterator nit = it;

		K.insert(*it);
		PowerSet(S, ++nit, K, PS, depth + 1, maxDepth);
		K.erase(*it);
	}
}

template<typename T> void PowerSet(
	const std::set<T>& S,
	const unsigned int maxDepth,
	std::vector< std::vector< std::set<T> > >& PS
) {
	if (S.empty()) {
		return;
	}

	std::set<T> K;

	// the power-set of S contains all subsets of size K = 1, 2, ..., |S|
	// (note there are <|S| over K> unique subsets of size K, ie. not both
	// {x, y} and {y, x} are included if S = {x, y, z})
	PS.resize(S.size() + 1);
	PowerSet(S, S.begin(), K, PS, 0, maxDepth);
}

#endif
