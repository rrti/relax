#ifndef RELAX_TYPES_HDR
#define RELAX_TYPES_HDR

// NOTE: the headers must match the typedefs
#include "learners/TDPolicy.hpp"
#include "learners/QLearning.hpp"
#include "tasks/SingleCorridorMaze.hpp"

namespace RELAX {
	// typedef Tasks::Dummy TTask;
	// typedef Tasks::HillClimber TTask;
	typedef Tasks::SingleCorridorMaze TTask;

	typedef TTask::State TState;
	typedef TTask::Action TAction;


	typedef Learners::QLearning<TState, TAction> Learner;
	// typedef Learners::SARSA<TState, TAction> Learner;

	// NOTE: TDPolicy::Learn only accepts TDLearnerBase instances!
	typedef Learners::TDPolicy<TState, TAction> Policy;
	// typedef Learners::GAPolicy<TState, TAction> Policy;
};

#endif

