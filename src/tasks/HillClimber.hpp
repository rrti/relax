#ifndef RELAX_HILLCLIMBER_TASK_HDR
#define RELAX_HILLCLIMBER_TASK_HDR

#include <cmath>
#include <vector>
#include <string>
#include "IAction.hpp"
#include "ITask.hpp"

#define HILL (HillClimber::GetInstance())
#define gTerrain (HILL.GetTerrain())
#define gVehicle (HILL.GetVehicle())

class LuaTable;
class INumberSequenceGen;

namespace RELAX {
	namespace Tasks {
		struct HillClimber: public ITask {
			struct Terrain {
				Terrain() { ga = cf = dx = a = b = 0.0f; }
				Terrain(float _ga, float _cf, float _dx,  float _a, float _b) {
					ga = _ga; cf = _cf; dx = _dx; a = _a; b = _b;
				}

				// if the derivative of f evaluated at x is dfx, then the 2D
				// tangent vector is <1.0, dfx> and the normal is <-dfx, 1.0>
				// (both non-unit length vectors)
				// NOTE: we only return the y-component of the tangent vector
				// (for Slope) and the x-component of the normal vector (for
				// Normal), the "terrain" is one-dimensional anyway
				// NOTE: the x-component of the normal has a sign, so gravity
				// acts two ways (if the slope is positive, it subtracts speed
				// so it becomes more negative along the x-axis and vice versa)
				// ==> if action is always IDLE, then velocity can ONLY decrease
				// (toward 0) when going up a slope, regardless of direction
				float Height(float x) const { return (a * cosf(x * b)); }
				float Slope(float x) const { return ((Height(x + dx) - Height(x)) / dx); }
				float Normal(float x) const { return -Slope(x); }

				// acceleration due to gravity is proportional to the length of the
				// unit-normal projected onto the x-axis (ie. simply the value of its
				// x-component)
				float UnitNormal(float x) const { return (Normal(x) / std::sqrt(Normal(x) * Normal(x) + 1.0f * 1.0f)); }
				float GravityAcceleration(float x) const { return (ga * UnitNormal(x)); }
				float FrictionCoefficient(float) const { return cf; }

				float MinPosition() const { return 0.0f; }
				float MaxPosition() const { return (float(M_PI + M_PI) / b); }

				float ClampPosition(float x) const { return std::max(MinPosition(), std::min(x, MaxPosition())); }
				bool PositionInBounds(float x) const { return ((x >= MinPosition()) && (x <= MaxPosition())); }


				float  a; // amplitude-scale: the domain of a * cos(...) is [-a, a]
				float  b; // frequency-scale: the period of cos(x * b) is (2PI / b)
				float dx; // step-size for x used to calculate the height derivative
				float ga; // constant of gravitational acceleration (m/s^2)
				float cf; // coefficient of static friction
			};


			struct Vehicle {
				Vehicle(): mass(0.0f), force(0.0f), vmin(0.0f), vmax(0.0f) {}
				Vehicle(float m, float f, float _vmin, float _vmax): mass(m), force(f), vmin(_vmin), vmax(_vmax) {}

				float EngineAcceleration() const { return (force / mass); }
				float MinVelocity() const { return vmin; }
				float MaxVelocity() const { return vmax; }

				float ClampVelocity(float v) const { return (std::max(MinVelocity(), std::min(v, MaxVelocity()))); }
				bool VelocityInBounds(float v) const { return ((v >= MinVelocity()) && (v <= MaxVelocity())); }

				float mass;  // in kg  (m = F/a)
				float force; // in N   (F = m*a)
				float vmin;  // in m/s
				float vmax;  // in m/s
			};


			struct Action: public IAction {
			public:
				enum {
					ACTION_POSX = 0, // right (+x)
					ACTION_NEGX = 1, // left (-x)
					ACTION_IDLE = 2, // idle
					NUM_ACTIONS = 3,
				};

				Action(unsigned int id = NUM_ACTIONS): IAction(id) {}

				static unsigned int GetMaxID() { return (NUM_ACTIONS - 1); }
				static unsigned int GetDefaultActionID() { return (NUM_ACTIONS - 1); }
				static unsigned int GetRandomActionID(unsigned int r) { return (r % NUM_ACTIONS); }
			};


			struct State {
			public:
				State();
				State(float pos, float vel);
				State& operator = (const State& state) {
					// RHS values are already clamped
					mPosition = state.mPosition;
					mVelocity = state.mVelocity;
					mID       = state.mID;
					return *this;
				}

			public:
				// functions required for RL
				State ApplyAction(const IAction& action, float* reward);

				State& Initialize(unsigned int sID);
				State& Randomize(INumberSequenceGen* nsg);

				unsigned int GetID() const { return mID; }
				static unsigned int GetMaxID();

				bool IsTerminal() const;
				bool operator < (const State& s) const { return (GetID() < s.GetID()); }

			public:
				// functions not required for RL
				float GetPosition() const { return mPosition; }
				float GetVelocity() const { return mVelocity; }

				static unsigned int GetPositionMult() { return gPositionMult; }
				static unsigned int GetVelocityMult() { return gVelocityMult; }
				static void SetPositionMult(unsigned int m) { gPositionMult = m; }
				static void SetVelocityMult(unsigned int m) { gVelocityMult = m; }

				float DistanceTo(const State& s) const {
					const float dPos = mPosition - s.mPosition;
					const float dVel = mVelocity - s.mVelocity;
					return std::sqrt((dPos * dPos) + (dVel * dVel));
				}
				std::string ToString() const;

			private:
				unsigned int CalculateID() const;

				// constants used to generate a discrete ID for each state
				// (together, these determine the size of the state-space)
				static unsigned int gPositionMult;
				static unsigned int gVelocityMult;

				float mPosition; // along the terrain x-axis
				float mVelocity; // along the terrain x-axis

				unsigned int mID;
			};


			static HillClimber& GetInstance() { static HillClimber hillClimber; return hillClimber; }
			static const char* GetName() { return "HillClimber"; }

			bool Initialize(const LuaTable* table);
			unsigned int GetChosenStates(std::vector<State>&, INumberSequenceGen*);

			const Terrain& GetTerrain() const { return mTerrain; }
			      Terrain& GetTerrain()       { return mTerrain; }
			const Vehicle& GetVehicle() const { return mVehicle; }
			      Vehicle& GetVehicle()       { return mVehicle; }
		private:
			Terrain mTerrain;
			Vehicle mVehicle;
		};
	}
}

#endif
