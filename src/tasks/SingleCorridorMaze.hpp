#ifndef RELAX_SINGLECORRIDORMAZE_TASK_HDR
#define RELAX_SINGLECORRIDORMAZE_TASK_HDR

#include <cmath>
#include <vector>
#include <string>
#include "IAction.hpp"
#include "ITask.hpp"

#define MAZE (SingleCorridorMaze::GetInstance())

class LuaTable;
class INumberSequenceGen;

namespace RELAX {
	namespace Tasks {
		struct SingleCorridorMaze: public ITask {
			struct Action: public IAction {
			public:
				enum {
					ACTION_LEFT  = 0,
					ACTION_RIGHT = 1,
					ACTION_UP    = 2, // always a no-op
					ACTION_DOWN  = 3, // always a no-op
					NUM_ACTIONS  = 2, // set to 4 if including UP/DOWN
				};

				Action(unsigned int id = NUM_ACTIONS): IAction(id) {}

				static unsigned int GetMaxID() { return (NUM_ACTIONS - 1); }
				static unsigned int GetDefaultActionID() { return (NUM_ACTIONS - 1); }
				static unsigned int GetRandomActionID(unsigned int r) { return (r % NUM_ACTIONS); }
			};


			struct State {
			public:
				State(): mRow(0), mCol(0), mID(CalculateID()) {}
				State& operator = (const State& state) {
					mRow = state.mRow;
					mCol = state.mCol;
					mID  = state.mID;
					return *this;
				}

			public:
				// functions required for RL
				State ApplyAction(const Action& action, float* reward);

				State& Initialize(unsigned int sID);
				State& Randomize(INumberSequenceGen* nsg);

				unsigned int GetID() const { return mID; }
				static unsigned int GetMaxID() { return (MAZE.GetNumRows() * MAZE.GetNumCols()) - 1; }

				bool IsTerminal() const { return (mCol == (MAZE.GetNumCols() - 1) && mRow == (MAZE.GetNumRows() - 1)); }
				bool operator < (const State& s) const { return (GetID() < s.GetID()); }

			public:
				float DistanceTo(const State& s) const {
					const float dx = float(mCol) - float(s.mCol);
					const float dy = float(mRow) - float(s.mRow);
					return std::sqrt((dx * dx) + (dy * dy));
				}
				std::string ToString() const;

			private:
				unsigned int CalculateID() const { return (mRow * MAZE.GetNumRows() + mCol); }

				unsigned int mRow; // y-coordinate
				unsigned int mCol; // x-coordinate
				unsigned int mID;
			};


			static SingleCorridorMaze& GetInstance() { static SingleCorridorMaze maze; return maze; }
			static const char* GetName() { return "SingleCorridorMaze"; }

			bool Initialize(const LuaTable*);
			unsigned int GetChosenStates(std::vector<State>&, INumberSequenceGen*);

			unsigned int GetNumRows() const { return mNumRows; }
			unsigned int GetNumCols() const { return mNumCols; }

		private:
			unsigned int mNumRows;
			unsigned int mNumCols;
		};
	}
}

#endif
