#include <cassert>
#include <cstdio>

#include "SingleCorridorMaze.hpp"
#include "../util/LuaParser.hpp"
#include "../util/INumberSequenceGen.hpp"

using namespace RELAX::Tasks;

bool SingleCorridorMaze::Initialize(const LuaTable* table) {
	assert(this == &MAZE);
	assert(!mInitialized);

	mNumRows = table->GetFltVal("numRows",  1.0f);
	mNumCols = table->GetFltVal("numCols", 10.0f);

	assert(mNumRows >= 1);
	assert(mNumCols >= 1);

	mRandomInitialStateActions = table->GetBoolVal("useRandomInitialStateActions", false);
	mRandomInitialActionValues = table->GetBoolVal("useRandomInitialActionValues", false);

	mInitialized = true;
	return true;
}

unsigned int SingleCorridorMaze::GetChosenStates(std::vector<State>& states, INumberSequenceGen*) {
	// pretend the probability distribution over states
	// has ALL mass concentrated at the left-most state,
	// so we would always draw it during learning etc.
	states.push_back(State());
	return (states.size());
}



SingleCorridorMaze::State& SingleCorridorMaze::State::Initialize(unsigned int sID) {
	sID  = std::min(sID, GetMaxID());
	mCol = sID % MAZE.GetNumCols();
	mRow = sID / MAZE.GetNumCols();
	mID  = sID;
	return *this;
}

SingleCorridorMaze::State& SingleCorridorMaze::State::Randomize(INumberSequenceGen* nsg) {
	do {
		mRow = nsg->NextInt() % MAZE.GetNumRows();
		mCol = nsg->NextInt() % MAZE.GetNumCols();
		mID  = CalculateID();
	} while (IsTerminal());

	assert(!IsTerminal());
	return *this;
}

SingleCorridorMaze::State SingleCorridorMaze::State::ApplyAction(const Action& action, float* reward) {
	State s = *this;
	*reward = -1.0f;

	switch (action.GetID()) {
		case Action::ACTION_LEFT:  {
			if (mCol > 0) {
				s.mCol -= 1;
			}
			// if this action is allowed, then the (left-most)
			// start-state does not subsume other states in S
			// (or rather, other states in part ALSO subsume
			// the space)
			// *reward = -1000.0f;
		} break;

		case Action::ACTION_RIGHT: {
			if (mCol < (MAZE.GetNumCols() - 1)) {
				s.mCol += 1;
			}
		} break;

		case Action::ACTION_UP:    { *reward = -5.0f; } break;
		case Action::ACTION_DOWN:  { *reward = -5.0f; } break;
	}

	if (s.IsTerminal()) {
		*reward = 1000.0f;
	}

	s.mID = s.CalculateID();
	return s;
}

std::string SingleCorridorMaze::State::ToString() const {
	static char buffer[128] = {'\0'};
	static const char* format = "<col=%u, row=%u>";

	sprintf(buffer, format, mCol, mRow);
	return buffer;
}
