#ifndef RELAX_DUMMY_TASK_HDR
#define RELAX_DUMMY_TASK_HDR

#include <vector>
#include <string>
#include "IAction.hpp"
#include "ITask.hpp"

class LuaTable;
class INumberSequenceGen;

namespace RELAX {
	namespace Tasks {
		struct Dummy: public ITask {
			struct Action: public IAction {
			public:
				enum {
					ACTION_NONE = 0,
					NUM_ACTIONS = 1,
				};

				Action(unsigned int id = NUM_ACTIONS): IAction(id) {}

				static unsigned int GetMaxID() { return (NUM_ACTIONS - 1); }
				static unsigned int GetDefaultActionID() { return (NUM_ACTIONS - 1); }
				static unsigned int GetRandomActionID(unsigned int r) { return (r % NUM_ACTIONS); }
			};


			struct State {
			public:
				State() {}
				State& operator = (const State&) { return *this; }

			public:
				// functions required for RL
				State ApplyAction(const Action&, float*) { return State(); }

				State& Initialize(unsigned int) { return *this; }
				State& Randomize(INumberSequenceGen*) { return *this; }

				unsigned int GetID() const { return 0; }
				static unsigned int GetMaxID() { return 0; }

				bool IsTerminal() const { return false; }
				bool operator < (const State& s) const { return (GetID() < s.GetID()); }

			public:
				float DistanceTo(const State&) const { return 0.0f; }
				std::string ToString() const;
			};


			static Dummy& GetInstance() { static Dummy dummy; return dummy; }
			static const char* GetName() { return "Dummy"; }

			bool Initialize(const LuaTable*) { return false; }

		private:
		};
	}
}

#endif
