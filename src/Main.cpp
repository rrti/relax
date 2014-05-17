#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <lua5.1/lua.hpp>

#include "Defines.hpp"
#include "Types.hpp"
#include "util/LuaParser.hpp"
#include "util/RandomNumberSequenceGen.hpp"
#include "util/StateSpaceGraph.hpp"
#include "util/PowerSet.hpp"

using namespace RELAX;



void UnitTest() {
	MTRandomNumberSequenceGen rng;
	TState s;

	for (unsigned int n = 0; n <= TState::GetMaxID(); n++) {
		s.Initialize(n);

		printf("[%s] n: %u, s: %s (ID: %u)\n", __FUNCTION__, n, (s.ToString()).c_str(), s.GetID());
		assert(n == s.GetID());
	}
}







bool InitializeBaseLineTest(
	const LuaTable* learnersTable,
	const LuaTable* policiesTable,
	TTask& task,
	std::vector<Policy>& randomPolicies,
	std::vector<Policy>& chosenPolicies,
	std::vector<Learner>& randomLearners,
	std::vector<Learner>& chosenLearners,
	std::vector<INumberSequenceGen*>& randomInitRNGs,
	std::vector<INumberSequenceGen*>& chosenInitRNGs,
	std::vector<INumberSequenceGen*>& randomEvalRNGs,
	std::vector<INumberSequenceGen*>& chosenEvalRNGs,
	bool weakBaseLine
) {
	printf("[%s]\n", __FUNCTION__);

	std::vector<TState> chosenStates;
	std::vector<TState> v;

	TState state;
	MTRandomNumberSequenceGen chosenRNG(random());

	if (task.GetChosenStates(chosenStates, &chosenRNG) == 0) {
		printf("[%s] task \"%s\" has not defined any chosen predictor states!\n", __FUNCTION__, TTask::GetName());
		return false;
	}

	for (unsigned int n = 0; n < randomPolicies.size(); n++) {
		Learners::TDLearnerParameters params;
		params.Initialize(learnersTable->GetTblVal("params"));
		params.SetRandomizeInitialStates(!weakBaseLine);

		state = state.Randomize(randomInitRNGs[n]);

		Learner learner(params);
		learner.SetInitialState(state);
		learner.SetNumberSequenceGen(randomEvalRNGs[n]);

		randomPolicies[n] = Policy(policiesTable);
		randomLearners[n] = learner;

		// initialize each learner's Q and each policy's PI
		// (either randomly or deterministically, according
		// to task-parameters)
		randomLearners[n].Initialize(randomInitRNGs[n], task.GetUseRandomInitialActionValues());
		randomPolicies[n].Initialize(randomInitRNGs[n], task.GetUseRandomInitialStateActions());
	}

	printf("\n");

	for (unsigned int n = 0; n < chosenPolicies.size(); n++) {
		// we do NOT want randomization of the initial states when
		// learning predictor-policies (because they are specially
		// chosen) regardless of whether test is weak or strong
		Learners::TDLearnerParameters params;
		params.Initialize(learnersTable->GetTblVal("params"));
		params.SetRandomizeInitialStates(false);

		state = chosenStates[chosenRNG.NextInt() % chosenStates.size()];

		// predictor-policies start being learned from predictor states
		Learner learner(params);
		learner.SetInitialState(state);
		learner.SetNumberSequenceGen(chosenEvalRNGs[n]);

		chosenPolicies[n] = Policy(policiesTable);
		chosenLearners[n] = learner;

		chosenLearners[n].Initialize(chosenInitRNGs[n], task.GetUseRandomInitialActionValues());
		chosenPolicies[n].Initialize(chosenInitRNGs[n], task.GetUseRandomInitialStateActions());
	}

	return true;
}



// weak baseline: each policy P is learned on ONE state (the
// same for all of P's learning episodes) and evaluated many
// times; each round evaluating P uses a different (random)
// starting state
//
// strong baseline: each policy P is learned on MANY states
// (newly randomized for each of P's learning episodes) and
// evalulated many times; each round evaluating P uses a
// different (random) starting state
//
void ExecuteBaseLineTest(
	std::vector<Policy>& randomPolicies,
	std::vector<Policy>& chosenPolicies,
	std::vector<Learner>& randomLearners,
	std::vector<Learner>& chosenLearners,
	std::vector<INumberSequenceGen*>& randomEvalRNGs,
	std::vector<INumberSequenceGen*>& chosenEvalRNGs,
	bool weakBaseLine
) {
	printf("[%s]\n", __FUNCTION__);

	const char* preTrialStr = "[%s] learning and evaluating %s%s policy %u (%u trial-rounds)\n";
	const char* pstTrialStr = "[%s] learned and evaluated %s%s policy %u (avg. trial-reward %.2f)\n\n";
	const char* testBaseName = weakBaseLine? "WEAK": "STRONG";

	// learn and evaluate the RANDOM policies
	for (unsigned int n = 0; n < randomPolicies.size(); n++) {
		Policy& policy = randomPolicies[n];
		Learner& learner = randomLearners[n];

		printf(preTrialStr, __FUNCTION__, testBaseName, "-RANDOM", n, policy.GetMaxEvaluationTrials());

		// learn and evaluate the n-th random policy
		policy.Learn(learner);
		policy.Evaluate(randomEvalRNGs[n]);

		printf(pstTrialStr, __FUNCTION__, testBaseName, "-RANDOM", n, 0.0f /*policy.GetTrialEpisodeRewardAvg()*/);
	}

	// learn and evaluate the CHOSEN (predictor) policies
	for (unsigned int n = 0; n < chosenPolicies.size(); n++) {
		Policy& policy = chosenPolicies[n];
		Learner& learner = chosenLearners[n];

		printf(preTrialStr, __FUNCTION__, testBaseName, "-CHOSEN", n, policy.GetMaxEvaluationTrials());

		// learn and evaluate the n-th chosen policy
		policy.Learn(learner);
		policy.Evaluate(chosenEvalRNGs[n]);

		printf(pstTrialStr, __FUNCTION__, testBaseName, "-CHOSEN", n, 0.0f /*policy.GetTrialEpisodeRewardAvg()*/);
	}
}



void SerializeBaseLineTestData(
	const LuaTable* mainTable,
	std::vector<Policy>& randomPolicies,
	std::vector<Policy>& chosenPolicies,
	bool weakBaseLine
) {
	const std::string dataDir = mainTable->GetStrVal("data", "./");
	const std::string taskName = TTask::GetName();
	const std::string testName = (weakBaseLine? "WEAK": "STRONG");

	assert(!dataDir.empty() && dataDir[dataDir.size() - 1] == '/');
	assert(!randomPolicies.empty() && !chosenPolicies.empty());
	assert(randomPolicies[0].GetMaxEvaluationTrials() == chosenPolicies[0].GetMaxEvaluationTrials());
	assert(randomPolicies[0].GetMaxLearningEpisodes() == chosenPolicies[0].GetMaxLearningEpisodes());

	std::vector<float> randomLearnerAvgTrainTrace(randomPolicies[0].GetMaxLearningEpisodes(), 0.0f);
	std::vector<float> chosenLearnerAvgTrainTrace(chosenPolicies[0].GetMaxLearningEpisodes(), 0.0f);
	std::vector<float> randomLearnerAvgTrialTrace(randomPolicies[0].GetMaxEvaluationTrials(), 0.0f);
	std::vector<float> chosenLearnerAvgTrialTrace(chosenPolicies[0].GetMaxEvaluationTrials(), 0.0f);

	std::stringstream learnerDataFileName;
	std::stringstream policyDataFileName;

	for (unsigned int n = 0; n < randomPolicies.size(); n++) {
		for (unsigned int k = 0; k < randomPolicies[n].GetMaxLearningEpisodes(); k++) {
			randomLearnerAvgTrainTrace[k] += randomPolicies[n].GetTrainEpisodeReward(k);
		}
		for (unsigned int k = 0; k < randomPolicies[n].GetMaxEvaluationTrials(); k++) {
			randomLearnerAvgTrialTrace[k] += randomPolicies[n].GetTrialEpisodeReward(k);
		}

		#ifdef RELAX_SERIALIZE_POLICY_DATA
		learnerDataFileName.str("");
		policyDataFileName.str("");
		learnerDataFileName << dataDir << "Q-RANDOM-" << taskName << "-" << n << "-" << testName << ".dat";
		policyDataFileName << dataDir << "PI-RANDOM-" << taskName << "-" << n << "-" << testName << ".dat";

		randomLearners[n].Serialize(learnerDataFileName.str());
		randomPolicies[n].Serialize(policyDataFileName.str());
		#endif

		#ifdef RELAX_SERIALIZE_LEARNER_TRACES
		learnerDataFileName.str("");
		learnerDataFileName << dataDir << "TRACE-RANDOM-" << taskName << "-" << n << "-" << testName << ".dat";

		// we just serialize the average trace over all
		// policies (not each individual policy trace)
		// randomLearners[n].SerializeEpisodeTraces(learnerDataFileName.str());
		#endif
	}


	for (unsigned int n = 0; n < chosenPolicies.size(); n++) {
		for (unsigned int k = 0; k < chosenPolicies[n].GetMaxLearningEpisodes(); k++) {
			chosenLearnerAvgTrainTrace[k] += chosenPolicies[n].GetTrainEpisodeReward(k);
		}
		for (unsigned int k = 0; k < chosenPolicies[n].GetMaxEvaluationTrials(); k++) {
			chosenLearnerAvgTrialTrace[k] += chosenPolicies[n].GetTrialEpisodeReward(k);
		}

		#ifdef RELAX_SERIALIZE_POLICY_DATA
		learnerDataFileName.str("");
		policyDataFileName.str("");
		learnerDataFileName << dataDir << "Q-CHOSEN-" << taskName << "-" << n << "-" << testName << ".dat";
		policyDataFileName << dataDir << "PI-CHOSEN-" << taskName << "-" << n << "-" << testName << ".dat";

		chosenLearners[n].Serialize(learnerDataFileName.str());
		chosenPolicies[n].Serialize(policyDataFileName.str());
		#endif

		#ifdef RELAX_SERIALIZE_LEARNER_TRACES
		learnerDataFileName.str("");
		learnerDataFileName << dataDir << "TRACE-CHOSEN-" << taskName << "-" << n << "-" << testName << ".dat";

		// we just serialize the average trace over all
		// policies (not each individual policy trace)
		// chosenLearners[n].SerializeEpisodeTraces(learnerDataFileName.str());
		#endif
	}


	{
		std::fstream f0; f0.open("random-train-avg.dat", std::ios::out);
		std::fstream f1; f1.open("chosen-train-avg.dat", std::ios::out);
		std::fstream f2; f2.open("random-trial-avg.dat", std::ios::out);
		std::fstream f3; f3.open("chosen-trial-avg.dat", std::ios::out);

		for (unsigned int k = 0; k < randomPolicies[0].GetMaxLearningEpisodes(); k++) {
			randomLearnerAvgTrainTrace[k] /= randomPolicies.size();
			chosenLearnerAvgTrainTrace[k] /= chosenPolicies.size();
		}
		for (unsigned int k = 0; k < randomPolicies[0].GetMaxEvaluationTrials(); k++) {
			randomLearnerAvgTrialTrace[k] /= randomPolicies.size();
			chosenLearnerAvgTrialTrace[k] /= chosenPolicies.size();
		}

		// output an "averaged" execution trace for all random and chosen
		// policies, statistics for both the training- and testing-phase
		//
		// note the results:
		//   random-train has much higher variance even after averaging
		//   than chosen-train (because its learning episodes can start
		//   from any state)
		//
		//   random-train obtains higher rewards than chosen-train: the
		//   chosen policies always start in state "0", which is furthest
		//   (1000 actions) away from goal and each non-terminal action
		//   has reward -1 --> episode reward can be -1 * 999 + 1000 at
		//   most once policy is optimal
		//
		//   chosen-trial reward is consistently higher than random-trial
		//   (this means the random policies are not optimal and need more
		//   learning episodes)
		for (unsigned int i = 0; i < randomPolicies[0].GetMaxLearningEpisodes(); i++) {
			f0 << i << "\t" << randomLearnerAvgTrainTrace[i] << "\n";
			f1 << i << "\t" << chosenLearnerAvgTrainTrace[i] << "\n";
		}
		for (unsigned int i = 0; i < randomPolicies[0].GetMaxEvaluationTrials(); i++) {
			f2 << i << "\t" << randomLearnerAvgTrialTrace[i] << "\n";
			f3 << i << "\t" << chosenLearnerAvgTrialTrace[i] << "\n";
		}

		f0.close();
		f1.close();
		f2.close();
		f3.close();
	}
}






int main(int argc, char** argv) {
	srandom(time(NULL));

	lua_State* luaState = NULL;
	LuaParser* luaParser = NULL;

	if (argc != 2) {
		printf("[%s] usage: %s <parameters.lua>\n", __FUNCTION__, argv[0]);
		return EXIT_FAILURE;
	}

	if ((luaState = lua_open()) != NULL) {
		// we need luaL_openlibs for math.random(),
		// which in turn depends on srandom() having
		// been called
		// luaL_openlibs(luaState);
		luaParser = new LuaParser(luaState);
	} else {
		return EXIT_FAILURE;
	}


	const bool parsed = luaParser->Execute(argv[1], true);
	const std::string& error = luaParser->GetError(argv[1]);

	if (!parsed) {
		printf("[%s] error: %s\n", __FUNCTION__, error.c_str());
		lua_close(luaState);
		delete luaParser;
		return EXIT_FAILURE;
	}

	const LuaTable*     rootTable = luaParser->GetRootTbl();
	const LuaTable*     mainTable = rootTable->GetTblVal(    "main");
	const LuaTable*     testTable = mainTable->GetTblVal(    "test");
	const LuaTable* learnersTable = rootTable->GetTblVal("learners");
	const LuaTable* policiesTable = rootTable->GetTblVal("policies");
	const LuaTable*    tasksTable = rootTable->GetTblVal(   "tasks");

	if (    mainTable == NULL) { printf("[%s]     mainTable: %p\n", __FUNCTION__,     mainTable); delete luaParser; return EXIT_FAILURE; }
	if (    testTable == NULL) { printf("[%s]     testTable: %p\n", __FUNCTION__,     testTable); delete luaParser; return EXIT_FAILURE; }
	if (learnersTable == NULL) { printf("[%s] learnersTable: %p\n", __FUNCTION__, learnersTable); delete luaParser; return EXIT_FAILURE; }
	if (policiesTable == NULL) { printf("[%s] policiesTable: %p\n", __FUNCTION__, policiesTable); delete luaParser; return EXIT_FAILURE; }
	if (   tasksTable == NULL) { printf("[%s]    tasksTable: %p\n", __FUNCTION__,    tasksTable); delete luaParser; return EXIT_FAILURE; }

	const        float fInitRNGSeed = mainTable->GetFltVal("initRNGSeed", -1.0f);
	const        float fEvalRNGSeed = mainTable->GetFltVal("evalRNGSeed", -1.0f);
	const unsigned int iInitRNGSeed = (fInitRNGSeed < 0.0f)? random(): (fInitRNGSeed * (1 << 31));
	const unsigned int iEvalRNGSeed = (fEvalRNGSeed < 0.0f)? random(): (fEvalRNGSeed * (1 << 31));

	const bool weakBaseLine = testTable->GetBoolVal("weakBaseLine", true);
	const unsigned int numRandomPolicies = static_cast<unsigned int>(testTable->GetFltVal("numRandomPolicies", 1)); // Nr
	const unsigned int numChosenPolicies = static_cast<unsigned int>(testTable->GetFltVal("numChosenPolicies", 1)); // Np


	printf("[%s]\n", __FUNCTION__);
	printf("  initRNGSeed(f): %f, evalRNGSeed(f): %f\n", fInitRNGSeed, fEvalRNGSeed);
	printf("  initRNGSeed(i): %u, evalRNGSeed(i): %u\n", iInitRNGSeed, iEvalRNGSeed);
	printf("\n");
	printf("  weakBaseLine:      %d\n",      weakBaseLine);
	printf("  numRandomPolicies: %u\n", numRandomPolicies);
	printf("  numChosenPolicies: %u\n", numChosenPolicies);
	printf("\n");


	std::vector<Policy> randomPolicies;
	std::vector<Policy> chosenPolicies;
	std::vector<Learner> randomLearners;
	std::vector<Learner> chosenLearners;

	// RNG's used to initialize PI and Q for each policy and learner, etc.
	//
	// if seeds are shared, then ALL initRNG's use the same seed N and ALL
	// evalRNG's use the same seed M (N and M need not be distinct values)
	// 
	std::vector<INumberSequenceGen*> randomInitRNGs;
	std::vector<INumberSequenceGen*> chosenInitRNGs;
	std::vector<INumberSequenceGen*> randomEvalRNGs;
	std::vector<INumberSequenceGen*> chosenEvalRNGs;

	{
		// NOTE: somewhat wasteful wrt. copying in InitializeBaseLineTest
		randomPolicies.resize(numRandomPolicies);
		chosenPolicies.resize(numChosenPolicies);
		randomLearners.resize(numRandomPolicies);
		chosenLearners.resize(numChosenPolicies);

		#ifdef RELAX_RNG_SHARED_SEEDS
		// give all init- and eval-RNG's the same initial seed (for debugging purposes)
		// this means every learner instance starts at the same random state, etc., so
		// the learned policies will be identical --> not generally useful
		for (unsigned int n = 0; n < randomPolicies.size(); n++) {
			randomInitRNGs.push_back(new MTRandomNumberSequenceGen(iInitRNGSeed));
			randomEvalRNGs.push_back(new MTRandomNumberSequenceGen(iEvalRNGSeed));
		}
		for (unsigned int n = 0; n < chosenPolicies.size(); n++) {
			chosenInitRNGs.push_back(new MTRandomNumberSequenceGen(iInitRNGSeed));
			chosenEvalRNGs.push_back(new MTRandomNumberSequenceGen(iEvalRNGSeed));
		}
		#else
		for (unsigned int n = 0; n < randomPolicies.size(); n++) {
			randomInitRNGs.push_back(new MTRandomNumberSequenceGen(random()));
			randomEvalRNGs.push_back(new MTRandomNumberSequenceGen(random()));
		}
		for (unsigned int n = 0; n < chosenPolicies.size(); n++) {
			chosenInitRNGs.push_back(new MTRandomNumberSequenceGen(random()));
			chosenEvalRNGs.push_back(new MTRandomNumberSequenceGen(random()));
		}
		#endif
	}


	// NOTE:
	//     this MUST be called before ANY state-instances
	//     are created, or the various constructors will
	//     see uninitialized data (by accessing *another*
	//     global task instance than the one initialized
	//     here)
	TTask& task = TTask::GetInstance();
	task.Initialize(tasksTable->GetTblVal(TTask::GetName()));

	printf("[%s] using learner \"%s\" for task \"%s\" (|S|: %u)\n", __FUNCTION__, Learner::GetName(), TTask::GetName(), TState::GetMaxID() + 1);

	if (InitializeBaseLineTest(
		learnersTable,
		policiesTable,
		task,
		randomPolicies,
		chosenPolicies,
		randomLearners,
		chosenLearners,
		randomInitRNGs,
		chosenInitRNGs,
		randomEvalRNGs,
		chosenEvalRNGs,
		weakBaseLine
	)) {
		ExecuteBaseLineTest(
			randomPolicies,
			chosenPolicies,
			randomLearners,
			chosenLearners,
			randomEvalRNGs,
			chosenEvalRNGs,
			weakBaseLine
		);

		SerializeBaseLineTestData(
			mainTable,
			randomPolicies,
			chosenPolicies,
			weakBaseLine);
	}

	for (unsigned int n = 0; n < randomPolicies.size(); n++) {
		delete randomInitRNGs[n];
		delete randomEvalRNGs[n];
	}
	for (unsigned int n = 0; n < chosenPolicies.size(); n++) {
		delete chosenInitRNGs[n];
		delete chosenEvalRNGs[n];
	}

	lua_close(luaState);
	delete luaParser; 
	return EXIT_SUCCESS;
}

