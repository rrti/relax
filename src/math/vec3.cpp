#include <cassert>
#include <cstdlib>
#include <string>
#include <sstream>

#include "vec3.hpp"
#include "../util/INumberSequenceGen.hpp"

// specialization for floats
template<> vec3<float>& vec3<float>::randomize(INumberSequenceGen* nsg) {
	x = nsg->NextFlt();
	y = nsg->NextFlt();
	z = nsg->NextFlt();
	return (inorm3D());
}

template<> std::string vec3<float>::str() const {
	std::string s;
	std::stringstream ss;
		ss << "<";
		ss << x << ", ";
		ss << y << ", ";
		ss << z;
		ss << ">";
	s = ss.str();
	return s;
}
