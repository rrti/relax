#ifndef RELAX_GALEARNERBASE_HDR
#define RELAX_GALEARNERBASE_HDR

#include <vector>

namespace RELAX {
	namespace Learners {
		template<typename TIndividual> class GALearnerBase {
		public:
			GALearnerBase() {
				mInitialized = false;

				mNumGenerations = 0;
				mMaxGenerations = 0;

				mNumberSeqGen = NULL;
			}

			GALearnerBase(const GALearnerParameters& parameters) {
				mInitialized = false;

				mNumGenerations = 0;
				mMaxGenerations = 0;

				mParameters = parameters;
				mNumberSeqGen = NULL;
			}

			GALearnerBase(const GALearnerBase& b) {
				*this = b;
			}

			GALearnerBase& operator = (const GALearnerBase& b) {
				// safe: operator= performs a deep-copy
				mPopulation = b.mPopulation;
				mFitnessValues = b.mFitnessValues;

				mInitialized = b.mInitialized;

				mNumGenerations = 0;
				mMaxGenerations = 0;

				mParameters = b.mParameters;
				mNumberSeqGen = b.mNumberSeqGen;
				return *this;
			}

			virtual ~GALearnerBase() {
				population.clear();
			}


			// initialize the population (of policy-individuals)
			// do not use our _own_ RNG to perform initialization
			void Initialize(INumberSequenceGen* nsg, bool randomize) {
				assert(!mInitialized);
				mPopulation.resize(mParameters.GetPopulationPoolSize());
				mFitnessValues.resize(mParameters.GetPopulationPoolSize(), 0.0f);

				for (unsigned int n = 0; n < population.size(); n++) {
					mPopulation[n].Initialize(nsg, randomize);
				}

				mInitialized = true;
			}

			void Evolve() {
				// evolve population until generation-limit reached or
				// average population fitness either no longer improves
				// or exceeds a preset minimum
				//
				// TODO: what should the population size even be?
				// TODO: how many candidate individuals do we pick each round?
				// TODO: of those candidates, how many pairs do we form?
				// NOTE: mParameters.GetCandidatePoolSize() should be < mParameters.GetPopulationPoolSize()
				std::vector<TIndividual*> candidates(mParameters.GetCandidatePoolSize(), NULL);

				while ((mNumGenerations++) < mMaxGenerations) {
					EvaluatePopulationFitness();
					SelectIndividuals(candidates);
					ReproduceIndividuals(candidates);
					EvolvePopulation();
				}
			}

		private:
			// called each time a round (generation) ends; used
			// to select which individuals get to breed and form
			// part of the next generation
			void EvaluatePopulationFitness(INumberSequenceGen* nsg) {
				for (unsigned int n = 0; n < population.size(); n++) {
					mFitnessValues[n] = mPopulation[n].Evaluate(mInitialState.Randomize(nsg));
				}
			}

			// select a proportion of the existing population to breed
			// a new generation through a fitness-based process (fitter
			// solutions as measured by the fitness function have bigger
			// chances of being selected)
			void SelectIndividuals(std::vector<TIndividual*>& candidates) {
				switch (mSelectionType) {
					case GA_SELECTION_ROULETTE: {
						// TODO: use mNumberSeqGen here
						for (unsigned int n = 0; n < mParameters.GetCandidatePoolSize(); n++) {
							candidates[n] = &mPopulation[123];
						}
					} break;
				}
			}

			void ReproduceIndividuals(std::vector<TIndividual*>& candidates) {
				// TODO: must form a (fixed?) number of <father, mother> pairs from candidates
			}

		protected:
			std::vector<TIndividual> mPopulation;
			std::vector<float> mFitnessValues;

			// true IFF Initialize was called
			bool mInitialized;

			// number of generations evolved so far
			unsigned int mNumGenerations;
			unsigned int mMaxGenerations;

			// stores all the parameter values used by this learner
			GALearnerParameters mParameters;
			TState mInitialState;

			INumberSequenceGen* mNumberSeqGen;
		};
	}
}

#endif

