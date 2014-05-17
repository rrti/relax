#ifndef RELAX_ITASK_HDR
#define RELAX_ITASK_HDR

class LuaTable;

namespace RELAX {
	namespace Tasks {
		struct ITask {
		public:
			ITask(): mInitialized(false) {}

			virtual bool Initialize(const LuaTable*) = 0;
			virtual bool GetUseRandomInitialStateActions() const { return mRandomInitialStateActions; }
			virtual bool GetUseRandomInitialActionValues() const { return mRandomInitialActionValues; }
			virtual void SetUseRandomInitialStateActions(bool b) { mRandomInitialStateActions = b; }
			virtual void SetUseRandomInitialActionValues(bool b) { mRandomInitialActionValues = b; }

		protected:
			bool mRandomInitialStateActions;
			bool mRandomInitialActionValues;
			bool mInitialized;
		};
	}
}

#endif

