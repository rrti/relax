#ifndef RELAX_TDLEARNERPARAMETERS_HDR
#define RELAX_TDLEARNERPARAMETERS_HDR

struct LuaTable;

namespace RELAX {
	namespace Learners {
		struct TDLearnerParameters {
		public:
			TDLearnerParameters() {
				mMaxActions = 0;

				mAlpha = 0.0f;
				mGamma = 0.0f;
				mEpsilon = 0.0f;

				mAlphaDecay = 0.0f;
				mEpsilonDecay = 0.0f;

				mMinAlpha = 0.0f;
				mMinEpsilon = 0.0f;

				mRandomizeInitialStates = false;
			}

			TDLearnerParameters(const TDLearnerParameters& p) {
				*this = p;
			}

			TDLearnerParameters& operator = (const TDLearnerParameters& p) {
				mMaxActions = p.mMaxActions;

				mAlpha = p.mAlpha;
				mGamma = p.mGamma;
				mEpsilon = p.mEpsilon;

				mAlphaDecay = p.mAlphaDecay;
				mEpsilonDecay = p.mEpsilonDecay;

				mMinAlpha = p.mMinAlpha;
				mMinEpsilon = p.mMinEpsilon;

				mRandomizeInitialStates = p.mRandomizeInitialStates;
				return *this;
			}

			bool Initialize(const LuaTable*);

			void SetMaxActions(unsigned int n) { mMaxActions = n; }
			void SetAlpha(float v) { mAlpha = v; }
			void SetGamma(float v) { mGamma = v; }
			void SetEpsilon(float v) { mEpsilon = v; }
			void SetAlphaDecay(float v) { mAlphaDecay = v; }
			void SetEpsilonDecay(float v) { mEpsilonDecay = v; }
			void SetMinAlpha(float v) { mMinAlpha = v; }
			void SetMinEpsilon(float v) { mMinEpsilon = v; }
			void SetRandomizeInitialStates(bool b) { mRandomizeInitialStates = b; }

			unsigned int GetMaxActions() const { return mMaxActions; }
			float GetAlpha() const { return mAlpha; }
			float GetGamma() const { return mGamma; }
			float GetEpsilon() const { return mEpsilon; }
			float GetAlphaDecay() const { return mAlphaDecay; }
			float GetEpsilonDecay() const { return mEpsilonDecay; }
			float GetMinAlpha() const { return mMinAlpha; }
			float GetMinEpsilon() const { return mMinEpsilon; }
			bool GetRandomizeInitialStates() const { return mRandomizeInitialStates; }

		private:
			unsigned int mMaxActions;      // max. number of actions allowed to be executed per episode

			float mAlpha;                  // learning-rate
			float mGamma;                  // future-reward discount factor
			float mEpsilon;                // exploration vs. exploitation parameter (in time, start favoring the latter)

			float mAlphaDecay;             // mAlpha multiplier (in [0.0, 1.0]), applied per EPISODE
			float mEpsilonDecay;           // mEpsilon multiplier (in [0.0, 1.0]), applied per EPISODE

			float mMinAlpha;               // minimum value that <mAlpha> is allowed to decay to
			float mMinEpsilon;             // minimum value that <mEpsilon> is allowed to decay to

			bool mRandomizeInitialStates;  // whether episodes start from random states while learning policy
		};
	}
}

#endif
