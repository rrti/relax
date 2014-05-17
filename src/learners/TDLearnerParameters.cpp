#include <cstdio>

#include "TDLearnerParameters.hpp"
#include "../Defines.hpp"
#include "../util/LuaParser.hpp"

bool RELAX::Learners::TDLearnerParameters::Initialize(const LuaTable* table) {
	if (table == NULL) { return false; }

	SetMaxActions(static_cast<unsigned int>(table->GetFltVal("maxActions", 0.0f)));
	SetAlpha(table->GetFltVal("alpha", 0.0f));
	SetGamma(table->GetFltVal("gamma", 0.0f));
	SetEpsilon(table->GetFltVal("epsilon", 0.0f));
	SetAlphaDecay(table->GetFltVal("alphaDecay", 0.0f));
	SetEpsilonDecay(table->GetFltVal("epsilonDecay", 0.0f));
	SetMinAlpha(table->GetFltVal("minAlpha", 0.0f));
	SetMinEpsilon(table->GetFltVal("minEpsilon", 0.0f));
	SetRandomizeInitialStates(table->GetBoolVal("randomizeInitialEpisodeStates", true));

	#ifdef RELAX_LOG_PARAMETERS
	printf("[TDLearnerParameters::%s]\n", __FUNCTION__);
	printf("  maximum actions per episode: %u\n", mMaxActions);
	printf("  learning-rate (alpha): %f\n", mAlpha);
	printf("  discount-rate (gamma): %f\n", mGamma);
	printf("  exploration-rate (epsilon): %f\n", mEpsilon);
	printf("  alpha-decay multiplier: %f\n", mAlphaDecay);
	printf("  epsilon-decay multiplier: %f\n", mEpsilonDecay);
	printf("  randomize initial states: %d\n", mRandomizeInitialStates);
	#endif

	return true;
}
