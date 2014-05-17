#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "HillClimber.hpp"
#include "../util/LuaParser.hpp"
#include "../util/INumberSequenceGen.hpp"

using namespace RELAX::Tasks;

unsigned int HillClimber::State::gPositionMult = 0;
unsigned int HillClimber::State::gVelocityMult = 0;

bool HillClimber::Initialize(const LuaTable* table) {
	assert(this == &HILL);
	assert(!mInitialized);

	const LuaTable* terrainTable = table->GetTblVal("Terrain");
	const LuaTable* vehicleTable = table->GetTblVal("Vehicle");

	if (terrainTable == NULL) { return false; }
	if (vehicleTable == NULL) { return false; }

	gTerrain.a  = terrainTable->GetFltVal("amplitudeScale", 1.0f);
	gTerrain.b  = terrainTable->GetFltVal("frequencyScale", 1.0f);
	gTerrain.dx = terrainTable->GetFltVal("stepSize", 0.01f);
	gTerrain.ga = terrainTable->GetFltVal("gravity",  0.981f);
	gTerrain.cf = terrainTable->GetFltVal("friction", 0.9f);

	gVehicle.mass  = vehicleTable->GetFltVal("mass",  1.0f);
	gVehicle.force = vehicleTable->GetFltVal("force", 0.05f);
	gVehicle.vmin  = vehicleTable->GetFltVal("vmin", -5.0f);
	gVehicle.vmax  = vehicleTable->GetFltVal("vmax",  5.0f);

	State::SetPositionMult(table->GetFltVal("positionMult", 100.0f));
	State::SetVelocityMult(table->GetFltVal("velocityMult",  10.0f));

	assert(State::GetPositionMult() >= 1);
	assert(State::GetVelocityMult() >= 1);

	mRandomInitialStateActions = table->GetBoolVal("useRandomInitialStateActions", false);
	mRandomInitialActionValues = table->GetBoolVal("useRandomInitialActionValues", false);

	mInitialized = true;
	return true;
}

unsigned int HillClimber::GetChosenStates(std::vector<State>&, INumberSequenceGen*) {
	/*
	static const float posRangeVariance = (gTerrain.MaxPosition() - gTerrain.MinPosition()) * 0.125f;
	static const float velRangeVariance = (gVehicle.MaxVelocity() - gVehicle.MinVelocity()) * 0.125f;

	assert(!states.empty());

	for (unsigned int n = 0; n < states.size(); n++) {
		// NOTE: each state is already default-initialized, but see [2] in Main
		states[n] = State();

		const float tau1 = nsg->NextFlt();
		const float tau2 = nsg->NextFlt();
		const float sgn1 = ((tau1 >= 0.5f)? 1.0f: -1.0f);
		const float sgn2 = ((tau2 >= 0.5f)? 1.0f: -1.0f);
		// NOTE:
		//     should we really restrict velocity this way? if we do, then the
		//     agent has to take MORE actions (compared to when starting from
		//     MOST random states) to reach a goal-state, but otherwise it gets
		//     a free advantage and obviously earns higher-than-average reward
		//     (only states close to the domain end-points would be even better)
		const float pos = states[n].GetPosition() + (tau1 * posRangeVariance * sgn1);
		const float vel = states[n].GetVelocity() + (tau2 * velRangeVariance * sgn2);

		states[n] = State(pos, vel);
	}
	*/
	return 0;
}



HillClimber::State::State() {
	// initialize position to the center of the valley, velocity to zero
	mPosition = gTerrain.MinPosition() + ((gTerrain.MaxPosition() - gTerrain.MinPosition()) * 0.5f);
	mVelocity = gVehicle.MinVelocity() + ((gVehicle.MaxVelocity() - gVehicle.MinVelocity()) * 0.5f);
	mID       = CalculateID();
}
HillClimber::State::State(float pos, float vel): mPosition(pos), mVelocity(vel) {
	mPosition = gTerrain.ClampPosition(mPosition);
	mVelocity = gVehicle.ClampVelocity(mVelocity);
	mID       = CalculateID();
}



HillClimber::State& HillClimber::State::Initialize(unsigned int sID) {
	static const unsigned int posRange = (gTerrain.MaxPosition() - gTerrain.MinPosition()) * gPositionMult;
	static const float epsilon = 0.001f;

	sID = std::min(sID, GetMaxID());
	mID = sID;

	const unsigned int pos = mID % posRange;
	const unsigned int vel = mID / posRange;

	mPosition = gTerrain.MinPosition() + ((float(pos) / gPositionMult) + epsilon);
	mVelocity = gVehicle.MinVelocity() + ((float(vel) / gVelocityMult) + epsilon);
	return *this;
}

HillClimber::State& HillClimber::State::Randomize(INumberSequenceGen* nsg) {
	static const float minPos = gTerrain.MinPosition(), maxPos = gTerrain.MaxPosition();
	static const float minVel = gVehicle.MinVelocity(), maxVel = gVehicle.MaxVelocity();

	do {
		mPosition = minPos + (nsg->NextFlt() * (maxPos - minPos));
		mVelocity = minVel + (nsg->NextFlt() * (maxVel - minVel));
	} while (IsTerminal());

	assert(!IsTerminal());
	return *this;
}

HillClimber::State HillClimber::State::ApplyAction(const IAction& action, float* reward) {
	float actionSign = 0.0f;

	// assign slightly bigger negative reward to idling
	switch (action.GetID()) {
		case Action::ACTION_POSX: { *reward = -1.0f; actionSign =  1.0f; } break;
		case Action::ACTION_NEGX: { *reward = -1.0f; actionSign = -1.0f; } break;
		case Action::ACTION_IDLE: { *reward = -2.0f; } break;
		default: { assert(false); } break;
	}

	const float accGravity = gTerrain.GravityAcceleration(mPosition);
	const float accEngine = gVehicle.EngineAcceleration() * actionSign;

	// copy the current state and modify it
	// NOTE: accEngine assumes "100% sticky tires" on slopes
	State s = *this;
	s.mVelocity += accEngine;
	s.mVelocity += accGravity;
	s.mVelocity *= gTerrain.FrictionCoefficient(mPosition);
	s.mVelocity  = gVehicle.ClampVelocity(s.mVelocity);
	s.mPosition += s.mVelocity;
	s.mPosition  = gTerrain.ClampPosition(s.mPosition);
	s.mID        = s.CalculateID();

	if (s.IsTerminal()) {
		*reward = 1000.0f;
	}

	return s;
}



unsigned int HillClimber::State::CalculateID() const {
	static const unsigned int posRange = (gTerrain.MaxPosition() - gTerrain.MinPosition()) * gPositionMult;
	static const float epsilon = 0.001f;

	// convert position and velocity to integer representation
	// for example, if <gPositionMult> is 100, this scales the
	// floating-point range [0.0, 2PI] to [0, 628] (giving 629
	// discrete integer positions)
	// HACK: add 0.001f to deal with numerical inaccuracy (if
	// we initialize a state <s> from ID <n=1>, then s.GetID()
	// should equal <n=1>, but this is not guaranteed due to
	// the FP-division in ::Initialize)
	const unsigned int pos = ((mPosition - gTerrain.MinPosition()) * gPositionMult) + epsilon;
	const unsigned int vel = ((mVelocity - gVehicle.MinVelocity()) * gVelocityMult) + epsilon;

	return (vel * posRange + pos);
}

unsigned int HillClimber::State::GetMaxID() {
	static const State maxState(gTerrain.MaxPosition(), gVehicle.MaxVelocity());
	static const unsigned int maxID(maxState.GetID());

	return maxID;
}

bool HillClimber::State::IsTerminal() const {
	// both position and velocity are individually clamped,
	// so only their combination can ever be out of bounds
	return !gTerrain.PositionInBounds(mPosition + mVelocity);
}



std::string HillClimber::State::ToString() const {
	static char buffer[128] = {'\0'};
	static const char* format = "<x=%.2f, v=%.2f>";

	sprintf(buffer, format, mPosition, mVelocity);
	return buffer;
}
