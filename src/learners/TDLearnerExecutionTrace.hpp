#ifndef RELAX_TDLEARNER_EXECUTIONTRACE_HDR
#define RELAX_TDLEARNER_EXECUTIONTRACE_HDR

#include <list>

struct TDLearnerExecutionTrace {
	TDLearnerExecutionTrace() {
		mNumActionRewards = 0;
		mCumActionReward = 0.0f;
		mAvgActionReward = 0.0f;
	}
	~TDLearnerExecutionTrace() {
		mActionRewards.clear();
	}

	void AddActionReward(float reward, bool append) {
		if (append) {
			// this is a memory hog, so do it only if requested
			mActionRewards.push_back(reward);
		}

		mNumActionRewards += 1;
		mCumActionReward += reward;
		mAvgActionReward = mCumActionReward / mNumActionRewards;
	}

	float GetCumulativeActionReward() const { return mCumActionReward; }
	float GetAverageActionReward() const { return mAvgActionReward; }

	const std::list<float>& GetActionRewards() const { return mActionRewards; }

private:
	// rewards for each executed action
	std::list<float> mActionRewards;

	// number of rewards received so far
	unsigned int mNumActionRewards;

	// total value of all rewards added so far
	float mCumActionReward;
	// running average of all rewards added so far
	float mAvgActionReward;
};

#endif

