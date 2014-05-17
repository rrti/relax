#ifndef RELAX_IACTION_HDR
#define RELAX_IACTION_HDR

namespace RELAX {
	namespace Tasks {
		struct IAction {
		public:
			IAction(): mID(-1U) {}
			IAction(unsigned int id): mID(id) {}
			IAction(const IAction& a) { *this = a; }
			IAction& operator = (const IAction& a) { mID = a.mID; return *this; }

			void SetID(unsigned int id) { mID = id; }
			unsigned int GetID() const { return mID; }

			virtual std::string ToString() const { return ""; }

		protected:
			unsigned int mID;
		};
	}
}

#endif
