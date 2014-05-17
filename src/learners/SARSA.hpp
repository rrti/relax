#ifndef RELAX_SARSA_HDR
#define RELAX_SARSA_HDR

#include "../Defines.hpp"
#include "TDLearnerBase.hpp"

namespace RELAX {
	namespace Learners {
		template<typename TState, typename TAction> class SARSA: public TDLearnerBase<TState, TAction> {
		public:
			SARSA() {}
			SARSA(const TDLearnerParameters& parameters): TDLearnerBase<TState, TAction>(parameters) {}
			SARSA(const SARSA& s): TDLearnerBase<TState, TAction>(s.mParameters) { *this = s; }
			SARSA& operator = (const SARSA& s) { TDLearnerBase<TState, TAction>::operator = (s); return *this; }

			static const char* GetName() { return "SARSA"; }

			float ExecuteEpisode(bool* status) {
				assert(this->mInitialized);

				TDLearnerParameters& params = this->mParameters;
				TDLearnerExecutionTrace trace;

				TState state = this->mInitialState;
				TAction action = SelectAction(state);

				if (params.GetRandomizeInitialStates())
					state.Randomize(this->mNumberSeqGen);

				#ifdef RELAX_LOG_LEARNER
				{
					const char* format = "[SARSA::%s] executing episode (state: %s, alpha: %f, epsilon: %f, gamma: %f)\n";
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

					const TState& sstate = state.ApplyAction(action, &actionReward);
					const TAction& aaction = SelectAction(sstate);

					ApplyUpdateRule(state, sstate, action, aaction, actionReward);
					trace.AddActionReward(actionReward, false);

					episodeReward += actionReward;
					actionReward = 0.0f;

					state = sstate;
					action = aaction;
				}

				params.SetAlpha(params.GetAlpha() * params.GetAlphaDecay());
				params.SetAlpha(std::max(params.GetAlpha(), params.GetMinAlpha()));
				params.SetEpsilon(params.GetEpsilon() * params.GetEpsilonDecay());
				params.SetEpsilon(std::max(params.GetEpsilon(), params.GetMinEpsilon()));

				if (status != NULL) {
					*status = (numActions < params.GetMaxActions());
				}

				return episodeReward;
			}

			void ApplyUpdateRule(const TState& s, const TState& ss, const TAction& a, const TAction& aa, float r) {
				const float alpha = this->mParameters.GetAlpha();
				const float gamma = this->mParameters.GetGamma();

				const float oldQsav   = GetActionValue(s, a);    // Q(s, a)
				const float oldQssaav = GetActionValue(ss, aa);  // Q(s', a')
				const float newQsav   = oldQsav + alpha * (r + gamma * oldQssaav - oldQsav);

				SetActionValue(s, a, newQsav);
			}
		};
	}
}

#endif
