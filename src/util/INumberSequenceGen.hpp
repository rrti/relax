#ifndef RELAX_I_NUMBER_SEQUENCE_GEN_HDR
#define RELAX_I_NUMBER_SEQUENCE_GEN_HDR

class INumberSequenceGen {
public:
	virtual ~INumberSequenceGen() {}

	virtual unsigned int NextInt() = 0;
	virtual double NextFlt() = 0;
};

#endif
