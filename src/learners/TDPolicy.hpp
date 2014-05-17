#ifndef RELAX_TDPOLICY_HDR
#define RELAX_TDPOLICY_HDR

#include "PolicyBase.hpp"
#include "TDLearnerBase.hpp"

namespace RELAX {
	namespace Learners {
		template<typename TState, typename TAction> class TDPolicy: public PolicyBase<TState, TAction> {
		public:
			TDPolicy(): PolicyBase<TState, TAction>() {}
			TDPolicy(const LuaTable* table): PolicyBase<TState, TAction>(table) {}
			TDPolicy(const TDPolicy& p): PolicyBase<TState, TAction>() { *this = p; }
			TDPolicy& operator = (const TDPolicy& p) { PolicyBase<TState, TAction>::operator = (p); return *this; }

			// a TDPolicy is learned through a TDLearner derivative
			// by having the learner execute a sequence of episodes
			// and using the learned action-values to decide which
			// action is taken for each state, forming the "optimal"
			// policy
			//
			// NOTE: only called ONCE per policy
			float Learn(TDLearnerBase<TState, TAction>& learner) {
				assert(this->mInitialized);
				assert(!this->mLearned);

				bool episodeTerminated = false;

				float episodeReward = 0.0f;
				float learnerReward = 0.0f;

				for (unsigned int n = 0; n < this->mMaxLearningEpisodes; n++) {
					episodeReward = learner.ExecuteEpisode(&episodeTerminated);
					learnerReward += episodeReward;

					this->mTrainEpisodeRewards[n] = episodeReward;
				}

				TState state;
				TAction action;

				// derive the optimal policy from the learned action-values
				for (unsigned int n = 0; n <= TState::GetMaxID(); n++) {
					state = state.Initialize(n);
					action = learner.GetBestAction(state);

					this->mStateActions[n] = action;
				}

				// make sure we aren't called again
				this->mLearned = true;
				return learnerReward;
			}
		};
	}
}

#endif
