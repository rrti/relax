#ifndef RELAX_TDLEARNERBASE_HDR
#define RELAX_TDLEARNERBASE_HDR

#include <vector>
#include <limits>

#include "TDLearnerParameters.hpp"
#include "TDLearnerExecutionTrace.hpp"
#include "../util/ISerializer.hpp"
#include "../util/INumberSequenceGen.hpp"

namespace RELAX {
	namespace Learners {
		template<typename TState, typename TAction> class TDLearnerBase: public ISerializer {
		public:
			TDLearnerBase(): ISerializer() {
				mInitialized = false;
				mNumberSeqGen = NULL;
			}

			TDLearnerBase(const TDLearnerParameters& parameters): ISerializer() {
				mInitialized = false;
				mParameters = parameters;
				mNumberSeqGen = NULL;
			}

			TDLearnerBase(const TDLearnerBase& b) {
				*this = b;
			}

			TDLearnerBase& operator = (const TDLearnerBase& b) {
				// safe: operator= performs a 2D deep-copy
				mActionValues = b.mActionValues;
				mInitialized = b.mInitialized;
				mParameters = b.mParameters;
				mInitialState = b.mInitialState;
				mNumberSeqGen = b.mNumberSeqGen;
				return *this;
			}

			virtual ~TDLearnerBase() {
				for (unsigned int n = 0; n < mActionValues.size(); n++) {
					mActionValues[n].clear();
				}

				mActionValues.clear();
			}

			// executes one episode
			virtual float ExecuteEpisode(bool* status) = 0;
			// performs one Q-value update for a single state-action
			// NOTE: <aa> is not used by the Q-learning update-rule
			virtual void ApplyUpdateRule(const TState& s, const TState& ss, const TAction& a, const TAction& aa, float r) = 0;


			void Initialize(INumberSequenceGen* nsg, bool randomize) {
				// NOTE:
				//     we assume each state shares the same set of actions
				//     if this is not the case, will need to refactor much
				//     (initialization and serialization code, etc.)
				// NOTE:
				//     do not use our _own_ RNG to set the action-values!
				assert(!mInitialized);
				mActionValues.resize(TState::GetMaxID() + 1, std::vector<float>(TAction::GetMaxID() + 1));

				for (unsigned int n = 0; n <= TState::GetMaxID(); n++) {
					for (unsigned int k = 0; k <= TAction::GetMaxID(); k++) {
						mActionValues[n][k] = randomize? nsg->NextFlt(): 0.0f;
					}
				}

				mInitialized = true;
			}

			void Serialize(const std::string& fileName) {
				unsigned int numStates = TState::GetMaxID() + 1;
				unsigned int numActions = TAction::GetMaxID() + 1;

				assert(mInitialized);
				assert(numStates == mActionValues.size());

				mSerializerFileStream.open(fileName.c_str(), std::ios::in | std::ios::binary);

				if (!mSerializerFileStream.good()) {
					// write the action-values
					mSerializerFileStream.close();
					mSerializerFileStream.open(fileName.c_str(), std::ios::out | std::ios::binary);

					mSerializerFileStream.write(reinterpret_cast<const char*>(&numStates), sizeof(unsigned int));
					mSerializerFileStream.write(reinterpret_cast<const char*>(&numActions), sizeof(unsigned int));

					for (unsigned int n = 0; n < numStates; n++) {
						for (unsigned int k = 0; k < numActions; k++) {
							mSerializerFileStream.write(reinterpret_cast<const char*>(&mActionValues[n][k]), sizeof(float));
						}
					}
				} else {
					// read the action-values
					mSerializerFileStream.read(reinterpret_cast<char*>(&numStates), sizeof(unsigned int));
					mSerializerFileStream.read(reinterpret_cast<char*>(&numActions), sizeof(unsigned int));
					mActionValues.resize(numStates, std::vector<float>(numActions, 0.0f));

					assert(numStates == (TState::GetMaxID() + 1));
					assert(numActions == (TAction::GetMaxID() + 1));

					for (unsigned int n = 0; n < numStates; n++) {
						for (unsigned int k = 0; k < numActions; k++) {
							mSerializerFileStream.read(reinterpret_cast<char*>(&mActionValues[n][k]), sizeof(float));
						}
					}
				}

				mSerializerFileStream.flush();
				mSerializerFileStream.close();
			}

			// return the size in bytes claimed by the action-value
			// table, excluding any internal data-structure overhead
			unsigned int GetSize() const { return ((TState::GetMaxID() + 1) * (TAction::GetMaxID() + 1) * sizeof(float)); }

			const TDLearnerParameters& GetParameters() const { return mParameters; }
			const TState& GetInitialState() const { return mInitialState; }

			void SetInitialState(const TState& s) { mInitialState = s; }
			void SetNumberSequenceGen(INumberSequenceGen* nsg) { assert(mNumberSeqGen == NULL); mNumberSeqGen = nsg; }

			TAction GetBestAction(const TState& s) {
				TAction a;
				GetMaxActionValue(s, a);
				return a;
			}

		protected:
			float GetActionValue(const TState& s, const TAction& a) const {
				assert(s.GetID() < mActionValues.size());
				assert(a.GetID() < mActionValues[s.GetID()].size());
				return mActionValues[s.GetID()][a.GetID()];
			}
			void SetActionValue(const TState& s, const TAction& a, float v) {
				assert(s.GetID() < mActionValues.size());
				assert(a.GetID() < mActionValues[s.GetID()].size());
				mActionValues[s.GetID()][a.GetID()] = v;
			}

			float GetMaxActionValue(const TState& s, TAction& a) const {
				assert(s.GetID() < mActionValues.size());

				float v = -std::numeric_limits<float>::max();

				const unsigned int id = s.GetID();
				const std::vector<float>& values = mActionValues[id];

				for (unsigned int n = 0; n < values.size(); n++) {
					if (values[n] > v) {
						v = values[n];
						a = n;
					}
				}

				return v;
			}

			TAction SelectAction(const TState& state) {
				TAction action;

				const float tau = mNumberSeqGen->NextFlt();
				const float epsilon = mParameters.GetEpsilon();

				// use epsilon-greedy strategy for action-selection
				// NOTE: the random action can still equal the best!
				if (tau >= epsilon) {
					GetMaxActionValue(state, action);
				} else {
					action.SetID(TAction::GetRandomActionID(mNumberSeqGen->NextInt()));
				}

				return action;
			}


			// first dimension: stateID, second dimension: actionID
			std::vector< std::vector<float> > mActionValues;

			// true IFF Initialize was called
			bool mInitialized;

			// stores all the parameter values used by this learner
			TDLearnerParameters mParameters;
			TState mInitialState;

			INumberSequenceGen* mNumberSeqGen;
		};
	};
}

#endif
