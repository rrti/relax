#ifndef RELAX_STATESPACEGRAPH_HDR
#define RELAX_STATESPACEGRAPH_HDR

#include <cassert>
#include <list>
#include <map>
#include <vector>

namespace RELAX {
	namespace Graphs {
		template<typename TState, typename TAction> class StateSpaceGraph {
		public:
			typedef std::vector<unsigned int> EdgeVec;
			typedef std::map<unsigned int, std::vector<unsigned int> > EdgeMap;

			StateSpaceGraph() { Initialize(); }
			StateSpaceGraph(const StateSpaceGraph& g) { *this = g; }

			~StateSpaceGraph() {
				for (EdgeMap::iterator it = mEdgeMap.begin(); it != mEdgeMap.end(); ++it) {
					(it->second).clear();
				}
				mEdgeMap.clear();
			}

			StateSpaceGraph& operator = (const StateSpaceGraph& g) {
				for (EdgeMap::const_iterator it = g.mEdgeMap.begin(); it != g.mEdgeMap.end(); ++it) {
					mEdgeMap[it->first] = it->second; // deep-copy
				}
			}



			// TODO: use for chokepoint detection (intermediate
			// states that are visited by a *majority* of paths)
			// NOTE: the shortest path has largest total reward
			//
			//     for each sourceState that !IsTerminal
			//         for each targetState that IsTerminal
			//             FindPath(sourceState, targetState, path)
			void FindPath(const TState& sourceStateNode, const TState& targetStateNode, std::list<TState>& pathNodes) {
			}


			void Initialize() {
				const unsigned int numStates = TState::GetMaxID() + 1;
				const unsigned int numActions = TAction::GetMaxID() + 1;

				TState sourceStateNode;
				TState targetStateNode;
				TAction sourceStateAction;

				float dummyReward = 0.0f;

				for (unsigned int n = 0; n < numStates; n++) {
					sourceStateNode.Initialize(n);
					assert(sourceStateNode.GetID() == n);

					mEdgeMap[n] = EdgeVec();

					if (sourceStateNode.IsTerminal()) {
						// no outgoing edges for terminal states
						continue;
					}

					mEdgeMap[n].resize(numActions);

					for (unsigned int k = 0; k < numActions; k++) {
						sourceStateAction = TAction(k);
						targetStateNode = sourceStateNode.ApplyAction(sourceStateAction, &dummyReward);

						mEdgeMap[sourceStateNode.GetID()][sourceStateAction.GetID()] = targetStateNode.GetID();
					}
				}
			}

			/*
			void Print() {
				TState sourceStateNode;
				TState targetStateNode;
				TAction sourceStateAction;

				for (EdgeMap::const_iterator mit = mEdgeMap.begin(); mit != mEdgeMap.end(); ++mit) {
					sourceStateNode.Initialize(mit->first);

					if (sourceStateNode.IsTerminal()) {
						continue;
					}

					const EdgeVec& edges = mit->second;

					// show all neighbor nodes (ie. every outgoing edge)
					for (unsigned int n = 0; n < edges.size(); n++) {
						sourceStateAction = TAction(n);
						targetStateNode.Initialize(edges[n]);
						printf("%s  --[%u]-->  %s (T: %d)\n", (sourceStateNode.ToString()).c_str(), n, (targetStateNode.ToString()).c_str(), targetStateNode.IsTerminal());
					}

					printf("\n");
				}
			}
			*/

			void GetReachableNodes(
				const float maxDistance,
				const TState& startStateNode,
				std::list<TState>& stateNodes
			) const {
				std::list<TState> stateNodeQueue;
				stateNodeQueue.push_front(startStateNode);

				std::vector<unsigned int> stateNodeCounts(TState::GetMaxID() + 1, 0);

				// show all nodes reachable (through one or more actions) from <s>
				while (!stateNodeQueue.empty()) {
					TState sourceStateNode = stateNodeQueue.front();
					TState targetStateNode;
					TAction sourceStateAction;

					const EdgeMap::const_iterator edgesIt = mEdgeMap.find(sourceStateNode.GetID());
					const EdgeVec& edges = edgesIt->second;

					assert(edgesIt != mEdgeMap.end());

					// guard against cycles
					if (stateNodeCounts[sourceStateNode.GetID()] > 0) {
						stateNodeQueue.pop_front();
						continue;
					}

					stateNodeQueue.pop_front();
					stateNodeCounts[sourceStateNode.GetID()] += 1;

					for (unsigned int n = 0; n < edges.size(); n++) {
						sourceStateAction = TAction(n);
						targetStateNode.Initialize(edges[n]);

						// skip self-edges and the original state
						if (targetStateNode.GetID() == sourceStateNode.GetID()) { continue; }
						if (targetStateNode.GetID() == startStateNode.GetID()) { continue; }

						// always add <targetState> to the queue: even if its distance
						// to <startState> exceeds <maxDistance>, further-down-the-chain
						// nodes that can be reached from it may be closer again
						//   push_front ==> depth-first exploration
						//   push_back ==> breadth-first exploration
						stateNodeQueue.push_front(targetStateNode);

						if (startStateNode.DistanceTo(targetStateNode) <= maxDistance) {
							stateNodes.push_back(targetStateNode);
						}
					}
				}
			}

			float GetAverageDensity(const float maxDistance) {
				const unsigned int numStates = TState::GetMaxID() + 1;

				unsigned int sumNgbStates = 0;
				unsigned int minNgbStates = numStates;
				unsigned int maxNgbStates = 0;

				TState stateNode;
				std::list<TState> stateNodes;

				// TODO: also calculate the variance
				for (unsigned int n = 0; n < numStates; n++) {
					stateNode.Initialize(n);
					GetReachableNodes(maxDistance, stateNode, stateNodes);

					if (!stateNode.IsTerminal()) {
						sumNgbStates += stateNodes.size();
						minNgbStates = std::min(minNgbStates, static_cast<unsigned int>(stateNodes.size()));
						maxNgbStates = std::max(maxNgbStates, static_cast<unsigned int>(stateNodes.size()));
					}

					stateNodes.clear();
				}

				return (sumNgbStates / float(numStates));
			}

		private:
			EdgeMap mEdgeMap;
		};
	}
}

#endif
