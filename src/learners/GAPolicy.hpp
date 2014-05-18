#ifndef RELAX_GAPOLICY_HDR
#define RELAX_GAPOLICY_HDR

namespace RELAX {
	namespace Learners {
		template<typename TState, typename TAction> class GALearnerBase;

		// NOTE:
		//     entire policy is one individual aka creature aka phenotype
		//     its state-action mapping is chromosome-string aka genotype
		template<typename TState, typename TAction> class GAPolicy: public PolicyBase {
		public:
			GAPolicy(): PolicyBase<TState, TAction>() {}
			GAPolicy(const LuaTable* table): PolicyBase<TState, TAction>(table) {}
			GAPolicy(const GAPolicy& p): PolicyBase<TState, TAction>() { *this = p; }
			GAPolicy& operator = (const GAPolicy& p) { PolicyBase<TState, TAction>::operator = (p); return *this; }


			GAPolicy& Mutate() {
				// TODO: implement a genetic algorithm for simple policy-search:
				//   1) maintain a population of policies
				//   2) apply genetic operators to individuals: mutation + crossover
				//   3) evaluate individual fitness: reward obtained by policy
				//   4) evolve until we are bored
				//
				// assume the decision to mutate has already been made somehow
				// pick one or more random? adjacent? <state=S, action=A> pairs
				// randomly? (which distribution?) change A to some action != A
				return *this;
			}

			GAPolicy& CrossOver(const GAPolicy& father, const GAPolicy& mother) {
				const std::vector<TState, TAction>& fatherGenes = father.GetGenes();
				const std::vector<TState, TAction>& motherGenes = mother.GetGenes();

				switch (mCrossOverType) {
					case GA_CROSSOVER_ONE_POINT: {} break;
					case GA_CROSSOVER_TWO_POINT: {} break;
					case GA_CROSSOVER_CUT_SPLICE: {} break;
					case GA_CROSSOVER_FULL_UNIFORM: {} break;
					case GA_CROSSOVER_HALF_UNIFORM: {} break;
				}

				return *this;
			}

			const std::vector<TState, TAction>& GetGenes() const { return (this->mStateActions); }
		};
	}
}

#endif

