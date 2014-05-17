#ifndef RELAX_POLICYBASE_HDR
#define RELAX_POLICYBASE_HDR

#include <vector>
#include "../Defines.hpp"
#include "../util/ISerializer.hpp"
#include "../util/INumberSequenceGen.hpp"
#include "../util/LuaParser.hpp"

namespace RELAX {
	namespace Learners {
		template<typename TState, typename TAction> class PolicyBase: public ISerializer {
		public:
			PolicyBase() {
				mInitialized = false;
				mLearned = false;
				mEvaluated = false;

				mMaxEvaluationTrials = 0;
				mMaxLearningEpisodes = 0;
				mMaxEpisodeActions = 0;
			}
			PolicyBase(const LuaTable* table): ISerializer() {
				mInitialized = false;
				mLearned = false;
				mEvaluated = false;

				mMaxEvaluationTrials = static_cast<unsigned int>(table->GetFltVal("maxEvaluationTrials", 0.0f));
				mMaxLearningEpisodes = static_cast<unsigned int>(table->GetFltVal("maxLearningEpisodes", 0.0f));
				mMaxEpisodeActions = static_cast<unsigned int>(table->GetFltVal("maxEpisodeActions", 0.0f));
			}

			PolicyBase(const PolicyBase& p): ISerializer() {
				*this = p;
			}

			PolicyBase& operator = (const PolicyBase& p) {
				// safe: operator= performs a 1D deep-copy
				mStateActions = p.mStateActions;
				mTrainEpisodeRewards = p.mTrainEpisodeRewards;
				mTrialEpisodeRewards = p.mTrialEpisodeRewards;

				mInitialized = p.mInitialized;
				mLearned = p.mLearned;
				mEvaluated = p.mEvaluated;

				mMaxEvaluationTrials = p.mMaxEvaluationTrials;
				mMaxLearningEpisodes = p.mMaxLearningEpisodes;
				mMaxEpisodeActions = p.mMaxEpisodeActions;

				return *this;
			}

			virtual ~PolicyBase() {
				mStateActions.clear();
				mTrainEpisodeRewards.clear();
				mTrialEpisodeRewards.clear();
			}



			void Initialize(INumberSequenceGen* nsg, bool randomize) {
				assert(!mInitialized);

				mStateActions.resize(TState::GetMaxID() + 1);
				mTrainEpisodeRewards.resize(mMaxLearningEpisodes, 0.0f);
				mTrialEpisodeRewards.resize(mMaxEvaluationTrials, 0.0f);

				// the policy should contain an action for every
				// possible state that can be encountered by the
				// agent (so we ensure this by pre-initializing)
				for (unsigned int n = 0; n <= TState::GetMaxID(); n++) {
					mStateActions[n] = randomize? TAction::GetRandomActionID(nsg->NextInt()): TAction::GetDefaultActionID();
				}

				mInitialized = true;
			}


			// NOTE: only called ONCE per policy
			float Evaluate(INumberSequenceGen* nsg) /*const*/ {
				assert(mInitialized);
				assert(mLearned);
				assert(!mEvaluated);

				float policyReward = 0.0f;

				for (unsigned int n = 0; n < mMaxEvaluationTrials; n++) {
					mTrialEpisodeRewards[n] = ExecuteEpisode(nsg);
					policyReward += mTrialEpisodeRewards[n];
				}

				mEvaluated = true;
				return policyReward;
			}


			void Serialize(const std::string& fileName) {
				assert(mInitialized);
				assert(mLearned);
				assert(mEvaluated);

				mSerializerFileStream.open(fileName.c_str(), std::ios::in | std::ios::binary);

				unsigned int numStates = TState::GetMaxID() + 1;
				unsigned int numActions = TAction::GetMaxID() + 1;

				if (!mSerializerFileStream.good()) {
					// write the state-actions
					mSerializerFileStream.close();
					mSerializerFileStream.open(fileName.c_str(), std::ios::out | std::ios::binary);

					mSerializerFileStream.write(reinterpret_cast<const char*>(&numStates), sizeof(unsigned int));
					mSerializerFileStream.write(reinterpret_cast<const char*>(&numActions), sizeof(unsigned int));

					for (unsigned int n = 0; n < numStates; n++) {
						mSerializerFileStream.write(reinterpret_cast<const char*>(&mStateActions[n]), sizeof(unsigned int));
					}
				} else {
					// read the state-actions
					mSerializerFileStream.read(reinterpret_cast<char*>(&numStates), sizeof(unsigned int));
					mSerializerFileStream.read(reinterpret_cast<char*>(&numActions), sizeof(unsigned int));
					mStateActions.resize(numStates);

					assert(numStates == (TState::GetMaxID() + 1));
					assert(numActions == (TAction::GetMaxID() + 1));

					for (unsigned int n = 0; n < numStates; n++) {
						mSerializerFileStream.read(reinterpret_cast<char*>(&mStateActions[n]), sizeof(unsigned int));
					}
				}

				mSerializerFileStream.flush();
				mSerializerFileStream.close();
			}

			// return the size in bytes claimed by the state-action
			// table, excluding any internal data-structure overhead
			unsigned int GetSize() const { return ((TState::GetMaxID() + 1) * sizeof(TAction)); }

			unsigned int GetMaxEvaluationTrials() const { return mMaxEvaluationTrials; }
			unsigned int GetMaxLearningEpisodes() const { return mMaxLearningEpisodes; }
			unsigned int GetMaxEpisodeActions() const { return mMaxEpisodeActions; }

			float GetTrainEpisodeReward(unsigned int k) const { return mTrainEpisodeRewards[k]; }
			float GetTrialEpisodeReward(unsigned int k) const { return mTrialEpisodeRewards[k]; }

		private:
			float ExecuteEpisode(INumberSequenceGen* nsg) {
				float episodeReward = 0.0f;
				float actionReward = 0.0f;

				TState state;

				// start each evaluation trial from a different random state
				state = state.Randomize(nsg);

				for (unsigned int k = 0; k < mMaxEpisodeActions; k++) {
					if (state.IsTerminal())
						break;

					assert(state.GetID() < mStateActions.size());

					const TAction& action = mStateActions[state.GetID()];
					const TState& sstate = state.ApplyAction(action, &actionReward);

					episodeReward += actionReward;
					state = sstate;
				}

				return episodeReward;
			}

		protected:
			// use a vector indexed by state ID's instead of
			// a map<TState, TAction> for faster lookups (at
			// the cost of increased memory use)
			std::vector<TAction> mStateActions;
			std::vector<float> mTrainEpisodeRewards;
			std::vector<float> mTrialEpisodeRewards;

			bool mInitialized;
			bool mLearned;
			bool mEvaluated;

			unsigned int mMaxEvaluationTrials;
			unsigned int mMaxLearningEpisodes;
			unsigned int mMaxEpisodeActions;
		};
	}
}

#endif

