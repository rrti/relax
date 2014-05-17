#ifndef RELAX_QLEARNING_HDR
#define RELAX_QLEARNING_HDR

#include <cassert>
#include <vector>

#include "../Defines.hpp"
#include "TDLearnerBase.hpp"

namespace RELAX {
	namespace Learners {
		template<typename TState, typename TAction> class QLearning: public TDLearnerBase<TState, TAction> {
		public:
			QLearning() {}
			QLearning(const TDLearnerParameters& parameters): TDLearnerBase<TState, TAction>(parameters) {}
			QLearning(const QLearning& q): TDLearnerBase<TState, TAction>(q.mParameters) { *this = q; }
			QLearning& operator = (const QLearning& q) { TDLearnerBase<TState, TAction>::operator = (q); return *this; }

			static const char* GetName() { return "QLearning"; }

			float ExecuteEpisode(bool* status) {
				// NOTE: need to dereference <this> explicitly here due to a
				// distinction between lookup of dependent and non-dependent
				// names (Serialize is not a dependent context, so the base
				// class is not known at compile-time)
				assert(this->mInitialized);

				TDLearnerParameters& params = this->mParameters;
				TDLearnerExecutionTrace trace;

				// copy the initial state so it does not get modified
				TState state = this->mInitialState;
				TAction aaction;

				if (params.GetRandomizeInitialStates())
					state.Randomize(this->mNumberSeqGen);

				#ifdef RELAX_LOG_LEARNER
				{
					const char* format = "[QLearning::%s] executing episode (state: %s, alpha: %f, epsilon: %f, gamma: %f)\n";
					const float alpha = params.GetAlpha();
					const float epsilon = params.GetEpsilon();
					const float gamma = params.GetGamma();

					printf(format, __FUNCTION__, (state.ToString()).c_str(), alpha, epsilon, gamma);
				}
				#endif

				float actionReward = 0.0f;
				float episodeReward = 0.0f;

				unsigned int numActions = 0;

				while (!state.IsTerminal()) {
					if ((numActions++) >= params.GetMaxActions())
						break;

					const TAction& action = SelectAction(state);
					const TState& sstate = state.ApplyAction(action, &actionReward);

					ApplyUpdateRule(state, sstate, action, aaction, actionReward);
					trace.AddActionReward(actionReward, false);

					episodeReward += actionReward;
					actionReward = 0.0f;

					state = sstate;
				}

				params.SetAlpha(params.GetAlpha() * params.GetAlphaDecay());
				params.SetAlpha(std::max(params.GetAlpha(), params.GetMinAlpha()));
				params.SetEpsilon(params.GetEpsilon() * params.GetEpsilonDecay());
				params.SetEpsilon(std::max(params.GetEpsilon(), params.GetMinEpsilon()));

				if (status != NULL) {
					*status = (numActions < params.GetMaxActions());
				}

				// NOTE:
				//   because almost all actions produce the same reward (-1),
				//   the average over all actions will be -1 too and this is
				//   not useful information to assess learner peformance
				//   (one exception is if the episode terminated because the
				//   goal-state was reached, but this is initially very rare)
				//   more interesting is the cumulative episode reward value,
				//   which should increase as the learner runs more episodes
				return episodeReward;
			}

		private:
			// NOTE:
			//   what if <ss> is equal to <s> due to the discretization scheme?
			//   does the update-rule still make any sense in such a situation?
			void ApplyUpdateRule(const TState& s, const TState& ss, const TAction& a, const TAction&, float r) {
				TAction maxQssa;

				const float alpha = this->mParameters.GetAlpha();
				const float gamma = this->mParameters.GetGamma();

				const float oldQsav  = GetActionValue(s, a);            // Q(s, a)
				const float maxQssav = GetMaxActionValue(ss, maxQssa);  // Q(s', a*)
				const float newQsav  = oldQsav + alpha * (r + gamma * maxQssav - oldQsav);

				SetActionValue(s, a, newQsav);
			}
		};
	}
}

#endif
